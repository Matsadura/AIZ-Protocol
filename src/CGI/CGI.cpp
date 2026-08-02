#include "CGI.hpp"
#include <cstddef>
#include <stdexcept>

CGI::CGI(const std::string &server_addr, const std::string &server_port, const std::string &client_addr,
         const std::string &client_port) :
    m_pipe_out(),
    m_pid(-1),
    m_start_time(0),
    m_client_addr(client_addr),
    m_client_port(client_port),
    m_server_addr(server_addr),
    m_server_port(server_port)
{
    m_pipe_out[0] = -1;
    m_pipe_out[1] = -1;
}

CGI::~CGI()
{
    // BUG: This will cause many problems when copying this object
    // Let the connection to handle CGI resources
    // waitAndClean();
}

int CGI::getOutFd() const
{
    return m_pipe_out[0];
}

pid_t CGI::getPid() const
{
    return m_pid;
}

void CGI::freeEnvArgv()
{
    for (size_t i = 0; i < m_envp.size(); ++i)
        free(m_envp[i]);
    m_envp.clear();

    for (size_t i = 0; i < m_argv.size(); ++i)
        free(m_argv[i]);
    m_argv.clear();
}

/**
 * execute - Execute the CGI script with the given request and script path
 * @req: The HTTP request
 * @scriptPath: The path to the CGI script
 */
void CGI::execute(const Request &req, const CgiMetaData &cgiMeta)
{
    if (pipe(m_pipe_out) == -1)
    {
        throw std::runtime_error("Failed to create pipe for CGI execution");
    }

    buildArgv(cgiMeta);
    buildEnv(req, cgiMeta);

    m_start_time = time(NULL);

    m_pid = fork();

    std::string working_directory;
    size_t      slash_pos = cgiMeta.script_path.find_last_of('/');
    if (slash_pos != std::string::npos)
    {
        working_directory = cgiMeta.script_path.substr(0, slash_pos);
    }

    if (m_pid < 0)
    {
        throw std::runtime_error("Failed to fork for CGI execution");
    }
    else if (m_pid == 0)
    {
        /* Child process */
        const std::string &body_file = req.getBodyFilename();

        int fd_in = -1;
        if (!body_file.empty())
        {
            LOG_INFO("CGI") << "Will read request body from: \"" << body_file << "\"\n";
            fd_in = open(body_file.c_str(), O_RDONLY);
        }

        if (fd_in == -1)
            fd_in = open("/dev/null", O_RDONLY);

        if (fd_in == -1)
        {
            LOG_ERROR("CGI") << "Failed to open request body file: " << body_file << "\n";
            std::exit(127);
        }

        dup2(fd_in, STDIN_FILENO);
        close(fd_in);

        dup2(m_pipe_out[1], STDOUT_FILENO);
        close(m_pipe_out[1]);
        close(m_pipe_out[0]);

        if (!working_directory.empty() && chdir(working_directory.c_str()) == -1)
        {
            LOG_ERROR("CGI") << "Failed to chdir to " << working_directory << "\n";
            std::exit(127);
        }

        std::string exec_path = cgiMeta.interpreter_path.empty() ? cgiMeta.script_path : cgiMeta.interpreter_path;
        execve(exec_path.c_str(), m_argv.data(), m_envp.data());
        std::exit(127);
    }
    else
    {
        /* Parent process */
        close(m_pipe_out[1]);
        m_pipe_out[1] = -1;

        int out_flags = fcntl(m_pipe_out[0], F_GETFL, 0);
        if (out_flags == -1)
            throw std::runtime_error("Failed to get flags for CGI output pipe");
        if (fcntl(m_pipe_out[0], F_SETFL, out_flags | O_NONBLOCK) == -1)
            throw std::runtime_error("Failed to set non-blocking flag for CGI output pipe");
        if (fcntl(m_pipe_out[0], F_SETFD, FD_CLOEXEC) == -1)
            throw std::runtime_error("Failed to set close-on-exec flag for CGI output pipe");
        LOG_INFO("CGI") << "EXECUTE=\"" << m_argv[0] << " " << m_argv[1] << "\" CWD=" << working_directory << "\n";
    }
}

/**
 * buildEnv - Build the environment variables for the CGI script execution
 * @req: The HTTP request
 * @cgiMeta: The CGI metadata
 */
void CGI::buildEnv(const Request &req, const CgiMetaData &cgiMeta)
{
    std::map<std::string, std::string> env_vars;

    env_vars["GATEWAY_INTERFACE"] = "CGI/1.1";
    env_vars["SERVER_SOFTWARE"]   = "AIZ/1.0";
    env_vars["REQUEST_METHOD"]    = req.getMethod();
    env_vars["REQUEST_URI"]       = req.getURI();
    env_vars["QUERY_STRING"]      = req.getQuery();
    env_vars["SERVER_PROTOCOL"]   = req.getVersion();
    env_vars["SCRIPT_NAME"]       = req.getPath();

    // For php-cgi
    env_vars["REDIRECT_STATUS"] = "200";
    env_vars["SCRIPT_FILENAME"] = m_argv[1];

    if (!cgiMeta.path_info.empty())
    {
        env_vars["PATH_INFO"]       = cgiMeta.path_info;
        env_vars["PATH_TRANSLATED"] = cgiMeta.path_info;
    }

    if (req.getHeaders().count("content-length"))
        env_vars["CONTENT_LENGTH"] = req.getHeader("content-length");
    if (req.getHeaders().count("content-type"))
        env_vars["CONTENT_TYPE"] = req.getHeader("content-type");

    std::string host_header = req.getHeader("host");
    size_t      colon_pos   = host_header.find(':');
    if (colon_pos != std::string::npos)
    {
        env_vars["SERVER_NAME"] = host_header.substr(0, colon_pos);
    }
    else
    {
        env_vars["SERVER_NAME"] = host_header;
    }

    env_vars["SERVER_ADDR"] = m_server_addr;
    env_vars["SERVER_PORT"] = m_server_port;
    env_vars["REMOTE_ADDR"] = m_client_addr;

    std::map<std::string, std::string> headers = req.getHeaders();
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
    {
        if (it->first == "content-length" || it->first == "content-type")
            continue;
        std::string header_key = "HTTP_" + it->first;
        std::replace(header_key.begin(), header_key.end(), '-', '_');
        std::transform(header_key.begin(), header_key.end(), header_key.begin(), ::toupper);
        env_vars[header_key] = it->second;
    }

    std::map<std::string, std::string>::iterator it;
    for (it = env_vars.begin(); it != env_vars.end(); ++it)
    {
        std::string env_entry = it->first + "=" + it->second;
        m_envp.push_back(strdup(env_entry.c_str()));
    }
    m_envp.push_back(NULL);
}

/**
 * buildArgv - Build the argument vector for the CGI script execution
 * @scriptPath: The path to the CGI script
 */
void CGI::buildArgv(const CgiMetaData &cgiMeta)
{
    if (!cgiMeta.interpreter_path.empty())
        m_argv.push_back(strdup(cgiMeta.interpreter_path.c_str()));

    std::string script_name = cgiMeta.script_path;
    size_t      slash_pos   = cgiMeta.script_path.find_last_of('/');

    if (slash_pos != std::string::npos)
    {
        script_name = cgiMeta.script_path.substr(slash_pos + 1, cgiMeta.script_path.size());
    }

    m_argv.push_back(strdup(script_name.c_str()));
    m_argv.push_back(NULL);
}

/**
 * waitAndClean - Wait for the CGI process to finish and clean up resources
 */
void CGI::waitAndClean()
{
    int status;
    reapZombie(status);

    if (m_pipe_out[0] != -1)
    {
        close(m_pipe_out[0]);
        m_pipe_out[0] = -1;
    }
    if (m_pipe_out[1] != -1)
    {
        close(m_pipe_out[1]);
        m_pipe_out[1] = -1;
    }

    freeEnvArgv();
}

bool CGI::isRunning() const
{
    if (m_pid <= 0)
        return false;

    int   status;
    pid_t result = waitpid(m_pid, &status, WNOHANG);
    if (result == 0)
        return true;
    else
        return false;
}

/**
 * True if more than @seconds have passed since the last updateStartTime() call
 */
bool CGI::isTimeout(time_t current_time, int seconds) const
{
    return (current_time - m_start_time) > seconds;
}

time_t CGI::getStartTime() const
{
    return m_start_time;
}

void CGI::updateStartTime()
{
    m_start_time = time(NULL);
}

/**
 * Check if cgi scripts exited with failure status
 *
 * Return: true if script failed
 */
bool CGI::exitedWithFailure(int status)
{
    bool failed = !WIFEXITED(status) || WEXITSTATUS(status) != 0;

    if (failed)
    {
        LOG_ERROR("CGI") << "Running: [\"" << m_argv[0] << "\" \"" << m_argv[1] << "\"] Failed ("
                         << describeStatus(status) << ")\n";
    }

    return failed;
}

/**
 * Capture the cgi status code to check if it failed
 */
void CGI::reapZombie(int &status)
{
    if (m_pid <= 0)
    {
        return;
    }

    pid_t result = waitpid(m_pid, &status, WNOHANG);
    if (result == m_pid)
    {
        m_pid = -1;
        return;
    }

    kill(m_pid, SIGKILL);
    LOG_INFO("CGI") << "Process with PID=" << m_pid << " has been killed\n";
    result = waitpid(m_pid, &status, 0);
    m_pid  = -1;
}

/**
 * This will be used to log what happened to the process if it failed
 */
std::string CGI::describeStatus(int status)
{
    std::ostringstream oss;

    if (WIFEXITED(status))
    {
        oss << "exited with code " << WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        oss << "killed by signal " << WTERMSIG(status);
    }
    else
    {
        oss << "ended abnormally";
    }
    return oss.str();
}
