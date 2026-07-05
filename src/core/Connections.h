#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include "../../src/Request/Request.hpp"
#include "Common.h"
class ListenerSocket;

class CGI;
class Multiplexer;

class Connections
{
    typedef struct
    {
        struct sockaddr_storage addr;
        int                     sockfd;
        int                     config_id;
        Request                 req;

        /* CGI related fields */

        bool   is_cgi;
        int    cgi_read_fd;
        int    cgi_write_fd;
        pid_t  cgi_pid;
        time_t cgi_start_time;

        size_t            cgi_bytes_written;
        std::vector<char> cgi_output_buffer;

        CGI *cgi_ptr;
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

    int           accept_new(int fd, Multiplexer &server);
    void          close_connection(int sockfd, Multiplexer &server);
    void          conn_handle_read(int sockfd, Multiplexer &server);
    connection_t &find(int sockfd);
};

#endif
