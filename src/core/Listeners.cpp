#include "Listeners.h"

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
    std::for_each(m_sockFds.begin(), m_sockFds.end(), close);
    m_sockFds.clear();
}

/**
 * Create a new listener socket, bind it to the specified address and port, and start listening for incoming connections
 */
void Listeners::create_new(const char *nodeName, const char *port)
{
    ListenerAddrInfo ai(nodeName, port);
    int sockfd;

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

    std::cout << "INFO: Listening at " << ai.toString() << "\n";
    std::cout << "INFO: " << sockfd << " file descriptor of listener socket\n";
    m_sockFds.push_back(sockfd);
}

/**
 * Check if the specified file descriptor is in the list of listener sockets
 *
 * @fd The file descriptor to check
 * @return true if the file descriptor is in the list of listener sockets, false otherwise
 */
bool Listeners::contains(int fd)
{
    return std::find(m_sockFds.begin(), m_sockFds.end(), fd) != m_sockFds.end();
}

/**
 * Get the number of listener servers
 */
std::size_t Listeners::size()
{
    return m_sockFds.size();
}

/**
 * Get the file descriptor of the listener socket at the specified index
 */
int Listeners::operator[](std::size_t index)
{
    return m_sockFds.at(index);
}
