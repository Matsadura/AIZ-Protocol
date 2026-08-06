#include "Connections.h"
#include "Multiplexer.h"

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

    struct sockaddr_storage server_addr     = {};
    socklen_t               server_addr_len = sizeof(server_addr);
    if (getsockname(listener_fd, reinterpret_cast<struct sockaddr *>(&server_addr), &server_addr_len) == -1)
    {
        int err = errno;
        throw std::runtime_error("getsockname() failed: " + std::string(std::strerror(err)));
    }

    struct sockaddr_storage conn_addr     = {};
    socklen_t               conn_addr_len = sizeof(conn_addr);

    int connection_fd = accept(listener_fd, reinterpret_cast<struct sockaddr *>(&conn_addr), &conn_addr_len);

    if (connection_fd == -1)
    {
        int err = errno;
        throw std::runtime_error(
            "Failed to accept new incoming client accept() failed: " + std::string(std::strerror(err)) + ")");
    }

    m_list[connection_fd] = connection_t(connection_fd, server_addr, conn_addr, listener_config);

    server.add_interest(m_list[connection_fd], Multiplexer::CLIENT, EPOLLIN);
    LOG_INFO("CONNECTIONS") << "New connection accepted (fd=" << connection_fd << ") from "
                            << addr_to_string(&conn_addr) << "\n";

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
    if (DebugStore::instance().enabled())
    {
        DebugStore::instance().dump(sockfd);
    }

    DebugStore::instance().erase(sockfd);

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

/**
 * Walks every open connection and kills the ones whose CGI process has been running too long
 */
void Connections::check_for_time_out()
{
    static time_t last_check_time = time(NULL);

    time_t current_time = time(NULL);

    if (current_time - last_check_time > TIME_BETWEEN_TIMEOUT_CHECKS)
    {
        last_check_time = current_time;
    }
    else
    {
        return;
    }

    std::map<int, connection_t>::iterator it = m_list.begin();
    while (it != m_list.end())
    {
        connection_t &conn = it->second;
        if (conn.cgi_active)
        {
            if (conn.cgi.isTimeout(current_time, CGI_TIMEOUT))
            {
                int unused = 0;
                it->second.cgi.reapZombie(unused); // This will force an event at the CGI_OUT descriptor
                conn.cgi_response.generateErrorResponse(504);
                LOG_INFO("CONNECTION") << "[fd=" << conn.sockfd << "] timeout expired\n";
            }
        }
        it++;
    }
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
