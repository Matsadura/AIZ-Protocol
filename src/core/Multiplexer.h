#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include "CGI_Response.hpp"
#include "Common.h"
#include "Connections.h"
#include "Listeners.h"

#define MAX_EVENTS 1024
#define BUFF_SIZE 65536

class Multiplexer
{
  private:
    int                m_epfd;
    struct epoll_event m_evlist[MAX_EVENTS];
    Connections        m_conns;
    Listeners          m_listeners;

    /**
     * @note: this class is uncopyable
     */
    Multiplexer(const Multiplexer &other);
    Multiplexer &operator=(const Multiplexer &other);

  public:
    /* what role the file descriptor plays within the connection*/
    enum FDRole
    {
        CLIENT,
        CGI_STDIN,
        CGI_STDOUT,
        LISTENER,
    };

    Multiplexer();
    ~Multiplexer();

    /**
     * Get the connection id for the current event
     *
     * @u64: the event.data.u64 event from epoll_wait()
     */
    static int unpack_conn_fd(uint64_t u64);

    /**
     * Get the role the fd plays within the connection
     *
     * @u64: the event.data.u64 event from epoll_wait()
     */
    static FDRole unpack_role(uint64_t u64);

    /*
     * Pack a connection fd and file descriptor where the event happend into a single 64-bit value for use with
     * epoll_event.data.u64
     */
    static uint64_t pack_data(int conn_fd, FDRole role);

    void run();
    void log_event(struct epoll_event ev);
    void epoll_apply(Connections::connection_t &conn, int op, FDRole role, uint32_t events);

    int get_role_fd(Connections::connection_t &conn, FDRole role)
    {
        switch (role)
        {
            case CLIENT:
                return conn.sockfd;
            case CGI_STDIN:
                return conn.cgi.getInFd();
            case CGI_STDOUT:
                return conn.cgi.getOutFd();
            default:
                UNREACHABLE("get_role_fd() got unknown role");
        }
    }

    uint32_t &get_role_events(Connections::connection_t &conn, FDRole role)
    {
        switch (role)
        {
            case CLIENT:
                return conn.sock_events;
            case CGI_STDIN:
                return conn.cgi_in_events;
            case CGI_STDOUT:
                return conn.cgi_out_events;
            default:
                UNREACHABLE("get_role_events() got unknown role");
        }
    }

    static const char *get_role_string(FDRole role)
    {
        switch (role)
        {
            case CLIENT:
                return "CLIENT";
            case LISTENER:
                return "LISTENER";
            case CGI_STDIN:
                return "CGI_STDIN";
            case CGI_STDOUT:
                return "CGI_STDOUT";
            default:
                UNREACHABLE("get_role_string() got unknown role");
        }
    }

    void add_interest(Connections::connection_t &conn, FDRole role, uint32_t events)
    {
        uint32_t &prev_events = get_role_events(conn, role);

        prev_events = events;
        epoll_apply(conn, EPOLL_CTL_ADD, role, events);
    }

    void modify_interest(Connections::connection_t &conn, FDRole role, uint32_t events)
    {
        uint32_t &prev_events = get_role_events(conn, role);
        if (prev_events != events)
        {
            prev_events = events;
            epoll_apply(conn, EPOLL_CTL_MOD, role, events);
        }
    }

    void remove_interest(Connections::connection_t &conn, FDRole role)
    {
        int       fd          = get_role_fd(conn, role);
        uint32_t &prev_events = get_role_events(conn, role);

        if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL) == 0)
        {
            prev_events = 0;
            LOG_INFO("EPOLL") << "Remove (fd=" << fd << ", type=" << get_role_string(role) << ")\n";
        }
        else
        {
            abort("epoll_ctl");
        }
    }

    void cgi_handle_in(Connections::connection_t &conn);
    void cgi_handle_out(Connections::connection_t &conn);

    void sock_handle_write(Connections::connection_t &conn);
    void sock_handle_read(Connections::connection_t &conn);

    void update_events(Connections::connection_t &conn);
};

#endif
