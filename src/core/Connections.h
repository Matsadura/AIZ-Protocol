#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include "Common.h"
class ListenerSocket;

class Multiplexer;

class Connections
{
    typedef struct
    {
        struct sockaddr_storage addr;
        int sockfd;
        int config_id;
    } connection_t;

  private:
    std::vector<connection_t> m_list;

    /**
     * @note: this class is uncopyable
     */
    Connections(const Connections &other);
    Connections &operator=(const Connections &other);

  public:
    Connections();
    ~Connections();

    int accept_new(int fd, Multiplexer &server);
    void close_connection(int sockfd, Multiplexer &server);
};

#endif
