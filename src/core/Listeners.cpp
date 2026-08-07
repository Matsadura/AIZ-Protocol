#include "Listeners.h"
#include <sstream>

int Listeners::PandingLimit = 1024;

Listeners::Listeners()
{
}

Listeners::Listeners(const Listeners &other) // NOLINT
{
    (void)other;
}

Listeners &Listeners::operator=(const Listeners &other) // NOLINT
{
    (void)other;
    return *this;
}

Listeners::~Listeners()
{
    for (std::map<int, s_Server>::iterator it = m_sockFds.begin(); it != m_sockFds.end(); ++it)
    {
        close(it->first);
    }
}

void Listeners::remove(int socket_fd)
{
    close(socket_fd);
    m_sockFds.erase(socket_fd);
}

static std::string create_report_messsage(const char *function_name, const std::string &node,
                                          const std::string &service, int error_code)
{
    // NOTE: Error prefix and last new line will be added in the place where this error will be caught
    // Furor: Failed to prepare listener socket for '0.0.0.0:8080'.
    // Cause: getaddrinfo() failed (Address already in use).
    std::ostringstream message;
    message << "Failed to prepare listener socket for '" << node << ":" << service << "'.\n";
    message << "    Cause: " << function_name << "() failed (" << std::strerror(error_code) << ").";
    return message.str();
}

/**
 * Create a new listener socket, bind it to the specified address and port, and start listening for incoming connections
 *
 * Return: file descriptor of the listening socket
 */
int Listeners::create_new(const std::string &node, const std::string &service, const s_Server &server)
{
    ListenerAddrInfo ai(node.c_str(), service.c_str());
    int              sockfd;

    sockfd = socket(ai.family(), ai.sockType(), 0);
    if (sockfd == -1)
    {
        close(sockfd);
        throw std::runtime_error(create_report_messsage("socket", node, service, errno));
    }
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        close(sockfd);
        throw std::runtime_error(create_report_messsage("setsockopt", node, service, errno));
    }

    if (bind(sockfd, ai.addr(), ai.addr_len()) != 0)
    {
        close(sockfd);
        throw std::runtime_error(create_report_messsage("bind", node, service, errno));
    }

    if (listen(sockfd, Listeners::PandingLimit) != 0)
    {
        close(sockfd);
        throw std::runtime_error(create_report_messsage("listen", node, service, errno));
    }

    LOG_INFO("LISTENERS") << "Listening at " << ai.toString() << " (fd=" << sockfd << ")\n";
    m_sockFds[sockfd] = server;
    return sockfd;
}

/**
 * Check if the specified file descriptor is in the list of listener sockets
 *
 * @fd The file descriptor to check
 * @return true if the file descriptor is in the list of listener sockets, false otherwise
 */
bool Listeners::contains(int fd)
{
    return m_sockFds.find(fd) != m_sockFds.end();
}

/**
 * Get the number of listener servers
 */
std::size_t Listeners::size()
{
    return m_sockFds.size();
}

s_Server *Listeners::get_listener_config(int fd)
{
    std::map<int, s_Server>::iterator it = m_sockFds.find(fd);

    if (it != m_sockFds.end())
    {
        return &(it->second);
    }
    return NULL;
}
