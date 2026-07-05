#include "Connections.h"
#include "../CGI/CGI.hpp"
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

    conn.is_cgi  = false;
    conn.cgi_ptr = NULL;

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

    if (m_list[sockfd].cgi_ptr != NULL)
    {
        m_list[sockfd].cgi_ptr->waitAndClean();
        delete m_list[sockfd].cgi_ptr;
        m_list[sockfd].cgi_ptr = NULL;
    }

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

        std::string uri = conn.req.getURI();
        /*wcript will be based on COnfig file later*/
        if (uri.find(".php") != std::string::npos || uri.find(".py") != std::string::npos)
        {
            LOG_INFO("REQUEST") << "[REQUEST] CGI routing (fd=" << sockfd << ")\n";
            conn.is_cgi            = true;
            conn.cgi_bytes_written = 0;
            conn.cgi_start_time    = time(NULL);

            conn.cgi_ptr = new CGI();

            std::string scriptPath = "." + uri; // Assuming the script is in the current directory

            conn.cgi_ptr->execute(conn.req, scriptPath);

            conn.cgi_read_fd  = conn.cgi_ptr->getReadFd();
            conn.cgi_write_fd = conn.cgi_ptr->getWriteFd();
            conn.cgi_pid      = conn.cgi_ptr->getPid();

            server.start_monitor_cgi(sockfd, conn.cgi_write_fd, Multiplexer::CGI_STDIN);
            server.start_monitor_cgi(sockfd, conn.cgi_read_fd, Multiplexer::CGI_STDOUT);
        }
        else
        {
            LOG_INFO("REQUEST") << "[REQUEST] Static file routing (fd=" << sockfd << ")\n";
            server.switch_conn_interest(sockfd, EPOLLOUT);
        }
    }
}

void Connections::conn_handle_cgi_write(int sockfd, Multiplexer &server)
{
    connection_t            &conn      = find(sockfd);
    const std::vector<char> &body      = conn.req.getBody();
    size_t                   remaining = body.size() - conn.cgi_bytes_written;

    if (remaining > 0)
    {
        ssize_t n = write(conn.cgi_write_fd, &body[0] + conn.cgi_bytes_written, remaining);

        if (n > 0)
        {
            conn.cgi_bytes_written += n;
            LOG_INFO("CGI") << "Wrote " << n << " bytes to script (fd=" << sockfd << ")\n";
        }
        else if (n == -1)
        {
            LOG_ERROR("CGI") << "Failed to write to CGI stdin (fd=" << sockfd << ")\n";
            close_connection(sockfd, server);
            return;
        }
    }

    if (conn.cgi_bytes_written >= body.size() && conn.cgi_write_fd != -1)
    {
        LOG_INFO("CGI") << "Finished writing to CGI stdin (fd=" << sockfd << ")\n";
        server.stop_monitor_conn(conn.cgi_write_fd);
        close(conn.cgi_write_fd);
        conn.cgi_write_fd = -1;
    }
}

void Connections::conn_handle_cgi_read(int sockfd, Multiplexer &server)
{
    connection_t &conn = find(sockfd);
    char          buffer[4096];
    ssize_t       n = read(conn.cgi_read_fd, buffer, sizeof(buffer));

    if (n > 0)
    {
        conn.cgi_output_buffer.insert(conn.cgi_output_buffer.end(), buffer, buffer + n);
        LOG_INFO("CGI") << "Read " << n << " bytes from CGI stdout (fd=" << sockfd << ")\n";
    }
    else if (n == 0)
    {
        conn_finish_cgi(sockfd, server);
    }
    else
    {
        LOG_ERROR("CGI") << "Failed to read from CGI stdout (fd=" << sockfd << ")\n";
        close_connection(sockfd, server);
        return;
    }
}

void Connections::conn_finish_cgi(int sockfd, Multiplexer &server)
{
    connection_t &conn = find(sockfd);

    if (!conn.is_cgi)
    {
        LOG_ERROR("CGI") << "Attempted to finish CGI on a non-CGI connection (fd=" << sockfd << ")\n";
        return;
    }

    LOG_INFO("CGI") << "CGI script finished (fd=" << sockfd << ")\n";

    if (conn.cgi_ptr != NULL)
    {
        if (conn.cgi_read_fd != -1)
        {
            server.stop_monitor_conn(conn.cgi_read_fd);
            close(conn.cgi_read_fd);
            conn.cgi_read_fd = -1;
        }
        conn.cgi_ptr->waitAndClean();
        delete conn.cgi_ptr;
        conn.cgi_ptr = NULL;
    }

    conn.is_cgi      = false;
    conn.cgi_read_fd = -1;
    server.switch_conn_interest(sockfd, EPOLLOUT);
}

Connections::~Connections()
{
    std::map<int, connection_t>::iterator it = m_list.begin();
    while (it != m_list.end())
    {
        if (it->second.cgi_ptr != NULL)
        {
            it->second.cgi_ptr->waitAndClean();
            delete it->second.cgi_ptr;
            it->second.cgi_ptr = NULL;
        }

        close(it->second.sockfd);
        it++;
    }
}