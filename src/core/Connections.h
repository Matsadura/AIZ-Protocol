#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include "../../src/Request/Request.hpp"
#include "../CGI/CGI.hpp"
#include "../CGI/CGIResponse.hpp"
#include "Common.h"

class ListenerSocket;
class Multiplexer;

class Connections
{
  public:
    typedef struct
    {
        struct sockaddr_storage addr;
        int                     sockfd;
        int                     config_id;
        Request                 req;
        uint32_t                sock_events;

        // CGI stuff
        bool        cgi_active;
        CGI         cgi;
        uint32_t    cgi_in_events;
        uint32_t    cgi_out_events;
        CGIResponse cgi_response;
        bool        closing;

    } connection_t;

  private:
    std::map<int, connection_t> m_list;

    /**
     * @note: this class is uncopyable
     */
    Connections(const Connections &other);
    Connections &operator=(const Connections &other);

  public:
    Connections();
    ~Connections();

    Connections::connection_t *find(int sockfd);
    int                        accept_new(int fd, Multiplexer &server);
    void                       close_connection(int sockfd, Multiplexer &server);
    void                       conn_handle_read(int sockfd, Multiplexer &server);
    void                       remove_read_intreset(connection_t &conn, Multiplexer &server);
    void                       add_read_intreset(connection_t &conn, Multiplexer &server);
    void                       remove_write_intreset(connection_t &conn, Multiplexer &server);
    void                       add_write_intreset(connection_t &conn, Multiplexer &server);
};

#endif
