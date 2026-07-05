#include "CGI.hpp"

CGI::CGI() : m_pid(-1)
{
    m_pipe_in[0]  = -1;
    m_pipe_in[1]  = -1;
    m_pipe_out[0] = -1;
    m_pipe_out[1] = -1;
}

CGI::~CGI()
{
    waitAndClean();
}

int CGI::getReadFd() const
{
    return m_pipe_out[0];
}

int CGI::getWriteFd() const
{
    return m_pipe_in[1];
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
void CGI::execute(const Request &req, const std::string &scriptPath)
{
    if (pipe(m_pipe_in) == -1 || pipe(m_pipe_out) == -1)
    {
        abort("Failed to create pipes for CGI execution");
    }

    buildEnv(req);
    buildArgv(scriptPath);

    m_pid = fork();
    if (m_pid < 0)
    {
        abort("Failed to fork for CGI execution");
    }
    else if (m_pid == 0)
    {
        /* Child process */
        dup2(m_pipe_in[0], STDIN_FILENO);
        dup2(m_pipe_out[1], STDOUT_FILENO);

        close(m_pipe_in[1]);
        close(m_pipe_out[0]);

        execve(scriptPath.c_str(), m_argv.data(), m_envp.data());
        abort("Failed to execute CGI script");
    }
    else
    {
        /* Parent process */
        close(m_pipe_in[0]);
        close(m_pipe_out[1]);

        fcntl(m_pipe_in[1], F_SETFL, O_NONBLOCK | FD_CLOEXEC);
        fcntl(m_pipe_out[0], F_SETFL, O_NONBLOCK | FD_CLOEXEC);
    }
}

/**
 * buildEnv - Build the environment variables for the CGI script execution
 * @req: The HTTP request
 */
void CGI::buildEnv(const Request &req)
{
    std::map<std::string, std::string> env_vars;

    env_vars["REQUEST_METHOD"]  = req.getMethod();
    env_vars["REQUEST_URI"]     = req.getURI();
    env_vars["QUERY_STRING"]    = req.getQuery();
    env_vars["SERVER_PROTOCOL"] = req.getVersion();

    if (req.getHeaders().count("Content-Length"))
    {
        env_vars["CONTENT_LENGTH"] = req.getHeader("Content-Length");
    }
    if (req.getHeaders().count("Content-Type"))
    {
        env_vars["CONTENT_TYPE"] = req.getHeader("Content-Type");
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
void CGI::buildArgv(const std::string &scriptPath)
{
    m_argv.push_back(strdup(scriptPath.c_str()));
    m_argv.push_back(NULL);
}

/**
 * waitAndClean - Wait for the CGI process to finish and clean up resources
 */
void CGI::waitAndClean()
{
    if (m_pid > 0)
    {
        int status;
        waitpid(m_pid, &status, WNOHANG);
        m_pid = -1;
    }

    if (m_pipe_in[0] != -1)
    {
        close(m_pipe_in[0]);
        m_pipe_in[0] = -1;
    }
    if (m_pipe_in[1] != -1)
    {
        close(m_pipe_in[1]);
        m_pipe_in[1] = -1;
    }
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