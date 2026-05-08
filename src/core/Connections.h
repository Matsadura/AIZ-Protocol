#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include "Common.h"
class ListenerSocket;

class Multiplexer;

class Connections : public Uncopyable
{
    typedef struct
    {
        struct sockaddr_storage addr;
        int sockfd;
        int config_id;
    } connection_t;

  private:
    std::vector<connection_t> m_list;

  public:
    Connections();
    Connections(const Connections &other);
    Connections &operator=(const Connections &other);
    ~Connections();

    int accept_new(int fd, Multiplexer &server);
    void close_connection(int sockfd, Multiplexer &server);
};

#endif
