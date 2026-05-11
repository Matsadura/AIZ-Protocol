#include "Connections.h"
#include "Multiplexer.h"

Connections::Connections()
{
}

Connections::Connections(const Connections &other) // NOLINT
{
    (void)other;
}

Connections &Connections::operator=(const Connections &other) // NOLINT
{
    (void)other;
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

Connections::~Connections()
{
    std::vector<connection_t>::iterator it = m_list.begin();
    while (it != m_list.end())
    {
        close((*it).sockfd);
        it++;
    }
}
