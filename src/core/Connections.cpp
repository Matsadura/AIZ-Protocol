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

int Connections::accept_new(int fd, Multiplexer &server)
{
    connection_t conn;
    socklen_t size = sizeof conn.addr;
    conn.sockfd    = accept(fd, reinterpret_cast<struct sockaddr *>(&conn.addr), &size);
    if (conn.sockfd == -1)
        abort("accept");
    std::cout << "INFO: [NEW_CONNECTION] (fd=" << conn.sockfd << ") "
              << addr_to_string(reinterpret_cast<struct sockaddr_in *>(&conn.addr)) << "\n";
    server.start_monitoring(conn.sockfd);
    m_list.push_back(conn);
    return conn.sockfd;
}

void Connections::close_connection(int sockfd, Multiplexer &server)
{
    std::cout << "INFO: [DISCONNECT] delete fd=" << sockfd << " connection\n";
    std::vector<connection_t>::iterator it = m_list.begin();
    while (it != m_list.end())
    {
        if ((*it).sockfd == sockfd)
        {
            server.stop_monitoring(sockfd);
            m_list.erase(it);
            close(sockfd);
            break;
        }
        it++;
    }
}

Connections::connection_t &Connections::find(int sockfd)
{
    for (std::vector<connection_t>::iterator it = m_list.begin(); it != m_list.end(); it++)
    {
        if ((*it).sockfd == sockfd)
            return *it;
    }
    UNREACHABLE("Epoll gives a none existing socket address, maybe forget to stop_monitoring it?");
}

void Connections::handle_read(int sockfd, Multiplexer &server)
{
    Connections::connection_t &conn = Connections::find(sockfd);
    char buff[4096];
    ssize_t n;

    n = recv(sockfd, buff, sizeof(buff), 0); // @todo: I don't know the use of 4th param!
    if (n == 0 || n == -1)
    {
        std::cout << "ERROR: [READ] read failed returns=" << n << " (fd=" << sockfd << ")\n";
        close_connection(sockfd, server);
        return;
    }
    std::cout << buff;
    conn.req.appendDataAndParse(buff, n);
    std::cout << "INFO: [REAQUEST] append " << n << " bytes to request\n";
    if (conn.req.getState() == Request::COMPLETE)
    {
        std::cout << "INFO: [REQUEST] parse completed\n";
        server.switch_interest(sockfd, EPOLLOUT);
    }
}

Connections::~Connections()
{
    std::vector<connection_t>::iterator it = m_list.begin();
    while (it != m_list.end())
    {
        close((*it).sockfd);
        it++;
    }
}
