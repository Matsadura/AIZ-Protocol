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
int Connections::accept_new(int fd, Multiplexer &server)
{
    connection_t conn;
    socklen_t    size = sizeof conn.addr;
    conn.sockfd       = accept(fd, reinterpret_cast<struct sockaddr *>(&conn.addr), &size);
    if (conn.sockfd == -1)
        abort("accept");
    LOG_INFO("CONNECTIONS") << "New connection accepted (fd=" << conn.sockfd << ") from "
                            << addr_to_string(reinterpret_cast<struct sockaddr_in *>(&conn.addr)) << "\n";
    m_list[conn.sockfd]   = conn;
    connection_t &cennRef = m_list[conn.sockfd];

    cennRef.cgi_in_events  = EPOLL_NOT_REGISTERED;
    cennRef.cgi_out_events = EPOLL_NOT_REGISTERED;
    cennRef.sock_events    = EPOLL_NOT_REGISTERED;
    cennRef.closing        = false;
    cennRef.cgi_active     = false;
    cennRef.config         = server.get_config(fd);
    cennRef.response       = NULL;
    if (cennRef.config == NULL)
    {
        UNREACHABLE("get_config should never return a null value\n");
    }
    server.add_interest(cennRef, Multiplexer::CLIENT, EPOLLIN);
    return conn.sockfd;
}

/**
 * Safely tear down a client session and stop tracking its events
 */
void Connections::close_connection(int sockfd, Multiplexer &server)
{
    connection_t *conn = find(sockfd);
    if (conn == NULL)
        return;

    if (conn->cgi_active)
    {
        server.remove_interest(*conn, Multiplexer::CGI_STDOUT);
        conn->cgi.waitAndClean(); // TODO: Mybe Kill the process?
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
        close(it->second.sockfd);
        it++;
    }
}
