#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include "../../src/Request/Request.hpp"
#include "../CGI/CGI.hpp"
#include "../CGI/CGIResponse.hpp"
#include "../Response/Response.hpp"
#include "../config_file_parser/parser/configfile.hpp"
#include "../config_file_parser/parser/directive.hpp"
#include "Common.h"

class ListenerSocket;
class Multiplexer;

#define EPOLL_NOT_REGISTERED 0xABCD

class Connections
{
  public:
    struct connection_t
    {
        struct sockaddr_storage addr;
        int                     sockfd;
        s_Server               *config;
        Response               *response;
        Request                 req;
        bool                    req_routed;
        uint32_t                sock_events;

        // CGI stuff
        bool        cgi_active;
        CGI         cgi;
        uint32_t    cgi_out_events;
        CGIResponse cgi_response;
        bool        closing;

        connection_t() :
            addr(),
            sockfd(),
            config(),
            response(),
            req_routed(),
            sock_events(),
            cgi_active(),
            cgi_out_events(),
            closing()
        {
        }

        connection_t(int conection_fd, const struct sockaddr_storage &client_addr, s_Server *server_config) :
            addr(client_addr),
            sockfd(conection_fd),
            config(server_config),
            response(NULL),
            req_routed(false),
            sock_events(EPOLL_NOT_REGISTERED),
            cgi_active(false),
            cgi_out_events(EPOLL_NOT_REGISTERED),
            closing(false)
        {
        }
    };

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
