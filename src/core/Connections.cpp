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
    server.start_monitor_conn(conn.sockfd);
    m_list[conn.sockfd] = conn;
    return conn.sockfd;
}

/**
 * Safely tear down a client session and stop tracking its events
 */
void Connections::close_connection(int sockfd, Multiplexer &server)
{
    LOG_INFO("CONNECTIONS") << "Close (fd=" << sockfd << ") connection\n";
    m_list.erase(sockfd);
    server.stop_monitor_conn(sockfd);
    close(sockfd);
}

/**
 * Grab a direct reference to an ongoing connection's data state
 */
Connections::connection_t &Connections::find(int sockfd)
{
    std::map<int, connection_t>::iterator it;
    it = m_list.find(sockfd);
    if (it != m_list.end())
    {
        return it->second;
    }
    UNREACHABLE("Epoll gives a none existing socket address, maybe forget to stop_monitoring it?");
}

/**
 * Read incoming data from a ready socket and feed it into the request parser
 *
 * @sockfd The file descriptor signaling that data is available to read
 * @server The multiplexer managing this connection's state
 */
void Connections::conn_handle_read(int sockfd, Multiplexer &server)
{
    Connections::connection_t &conn = Connections::find(sockfd);
    char                       buff[4096];
    ssize_t                    n;

    n = recv(sockfd, buff, sizeof(buff), 0); // @todo: I don't know the use of 4th param!
    if (n == 0)
    {
        LOG_INFO("CONNECTIONS") << "Read peer shutdown (fd=" << sockfd << ")\n";
        close_connection(sockfd, server);
        return;
    }
    else if (n == -1)
    {
        LOG_ERROR("CONNECTIONS") << "Read (recv) failed (fd=" << sockfd << ")\n";
        close_connection(sockfd, server);
        return;
    }
    conn.req.appendDataAndParse(buff, n);
    LOG_INFO("REQUEST") << n << " bytes appended (fd=" << sockfd << ")\n";
    LOG_INFO("REQUEST") << "State: " << conn.req.getState() << " (fd=" << sockfd << ")\n";
    if (conn.req.getState() == Request::COMPLETE)
    {
        LOG_INFO("REQUEST") << "[REQUEST] Parsing is completed (fd=" << sockfd << ")\n";
        server.switch_conn_interest(sockfd, EPOLLOUT);
    }
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