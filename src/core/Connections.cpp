#include "Connections.h"
#include "Multiplexer.h"
#include <cassert>

Connections::Connections()
{
}

Connections::Connections(const Connections &other) // NOLINT
{
    UNUSED(other);
}

Connections &Connections::operator=(const Connections &other) // NOLINT
{
    UNUSED(other);
    return (*this);
}

/**
 * Accept a fresh client connection and register it for monitoring
 */
int Connections::accept_new(int listener_fd, Multiplexer &server)
{
    s_Server *listener_config = server.get_config(listener_fd);
    if (!listener_config)
    {
        throw std::runtime_error("Couldn't find the server conifg associated with the listener");
    }

    struct sockaddr_storage addr          = {};
    socklen_t               addr_len      = sizeof(addr);
    int                     connection_fd = accept(listener_fd, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);

    if (connection_fd == -1)
    {
        int err = errno;
        throw std::runtime_error(
            "Failed to accept new incoming client accept() failed: " + std::string(std::strerror(err)) + ")");
    }

    m_list[connection_fd] = connection_t(connection_fd, addr, listener_config);

    server.add_interest(m_list[connection_fd], Multiplexer::CLIENT, EPOLLIN);
    LOG_INFO("CONNECTIONS") << "New connection accepted (fd=" << connection_fd << ") from "
                            << addr_to_string(reinterpret_cast<struct sockaddr_in *>(&addr)) << "\n";

    return connection_fd;
}

/**
 * Safely tear down a client session and stop tracking its events
 */
void Connections::close_connection(int sockfd, Multiplexer &server)
{
    connection_t *conn = find(sockfd);
    if (conn == NULL)
    {
        return;
    }

    if (conn->cgi_active)
    {
        server.remove_interest(*conn, Multiplexer::CGI_STDOUT);
        conn->cgi.waitAndClean();
    }

    LOG_INFO("CONNECTIONS") << "Close (fd=" << sockfd << ") connection\n";
    server.remove_interest(*conn, Multiplexer::CLIENT);
    close(sockfd);
    delete conn->response;
    m_list.erase(sockfd);
}

/**
 * Grab a direct reference to an ongoing connection's data state
 */
Connections::connection_t *Connections::find(int sockfd)
{
    std::map<int, connection_t>::iterator it;
    it = m_list.find(sockfd);
    return it == m_list.end() ? NULL : &it->second;
}

Connections::~Connections()
{
    std::map<int, connection_t>::iterator it = m_list.begin();
    while (it != m_list.end())
    {
        it->second.cgi.waitAndClean();
        close(it->second.sockfd);
        delete it->second.response;
        it++;
    }
}
