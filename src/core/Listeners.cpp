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

std::string int_to_string(int n)
{
    std::stringstream ss;

    ss << n;
    return ss.str();
}

/**
 * Create a new listener socket, bind it to the specified address and port, and start listening for incoming connections
 *
 * Return: file descriptor of the listening socket
 */
int Listeners::create_new(const s_Server &server)
{
    std::string node_name = server.ports.begin()->first;
    std::string port      = int_to_string(server.ports.begin()->second[0]);

    ListenerAddrInfo ai(node_name.c_str(), port.c_str());
    int              sockfd;

    sockfd  = socket(ai.family(), ai.sockType(), 0);
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    if (bind(sockfd, ai.addr(), ai.addr_len()) != 0)
        abort("bind");

    if (listen(sockfd, Listeners::PandingLimit) != 0)
        abort("lister");

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
