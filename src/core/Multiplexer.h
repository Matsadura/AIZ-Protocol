#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include "../CGI/CGIResponse.hpp"
#include "../Router/Router.hpp"
#include "../config_file_parser/parser/configfile.hpp"
#include "Common.h"
#include "Connections.h"
#include "Listeners.h"
#include <cstddef>
#include <sys/epoll.h>

#define MAX_EVENTS 100000
#define BUFF_SIZE 65536

#define __ANSI_SEQ(n) "\033[" #n "m"

#define BOLD __ANSI_SEQ(1)

#define COLOR_WEAK __ANSI_SEQ(2)
#define COLOR_HIGHLIGHT __ANSI_SEQ(3)
#define COLOR_UNDERLINE __ANSI_SEQ(4)
#define COLOR_BLACK __ANSI_SEQ(30)
#define COLOR_DARK_RED __ANSI_SEQ(31)
#define COLOR_DARK_GREEN __ANSI_SEQ(32)
#define COLOR_DARK_YELLOW __ANSI_SEQ(33)
#define COLOR_DARK_BLUE __ANSI_SEQ(34)
#define COLOR_DARK_PINK __ANSI_SEQ(35)
#define COLOR_DARK_CYAN __ANSI_SEQ(36)
#define COLOR_BLACK_BG __ANSI_SEQ(40)
#define COLOR_DARK_RED_BG __ANSI_SEQ(41)
#define COLOR_DARK_GREEN_BG __ANSI_SEQ(42)
#define COLOR_DARK_YELLOW_BG __ANSI_SEQ(43)
#define COLOR_DARK_BLUE_BG __ANSI_SEQ(44)
#define COLOR_DARK_PINK_BG __ANSI_SEQ(45)
#define COLOR_DARK_CYAN_BG __ANSI_SEQ(46)
#define COLOR_GRAY __ANSI_SEQ(90)
#define COLOR_LIGHT_RED __ANSI_SEQ(91)
#define COLOR_LIGHT_GREEN __ANSI_SEQ(92)
#define COLOR_LIGHT_YELLOW __ANSI_SEQ(93)
#define COLOR_LIGHT_BLUE __ANSI_SEQ(94)
#define COLOR_LIGHT_PINK __ANSI_SEQ(95)
#define COLOR_LIGHT_CYAN __ANSI_SEQ(96)
#define COLOR_LIGHT_GRAY __ANSI_SEQ(97)
#define COLOR_GRAY_BG __ANSI_SEQ(100)
#define COLOR_LIGHT_RED_BG __ANSI_SEQ(101)
#define COLOR_LIGHT_GREEN_BG __ANSI_SEQ(102)
#define COLOR_LIGHT_YELLOW_BG __ANSI_SEQ(103)
#define COLOR_LIGHT_BLUE_BG __ANSI_SEQ(104)
#define COLOR_LIGHT_PINK_BG __ANSI_SEQ(105)
#define COLOR_LIGHT_CYAN_BG __ANSI_SEQ(106)
#define COLOR_LIGHT_GRAY_BG __ANSI_SEQ(107)

#define RESET "\033[m"

class Multiplexer
{
  private:
    int                m_epfd;
    struct epoll_event m_evlist[MAX_EVENTS];
    Connections        m_conns;
    Listeners          m_listeners;
    ConfigFile         m_config;

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

    s_Server *get_config(int fd)
    {
        return m_listeners.get_listener_config(fd);
    }

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
                return COLOR_DARK_PINK "CLIENT" RESET;
            case LISTENER:
                return COLOR_DARK_PINK "LISTENER" RESET;
            case CGI_STDIN:
                return COLOR_DARK_PINK "CGI_STDIN" RESET;
            case CGI_STDOUT:
                return COLOR_DARK_PINK "CGI_STDOUT" RESET;
            default:
                UNREACHABLE("get_role_string() got unknown role");
        }
    }

    void add_interest(Connections::connection_t &conn, FDRole role, uint32_t events)
    {
        uint32_t &prev_events = get_role_events(conn, role);

        if (prev_events != EPOLL_NOT_REGISTERED)
        {
            return;
        }
        prev_events = events;
        epoll_apply(conn, EPOLL_CTL_ADD, role, events);
    }

    void modify_interest(Connections::connection_t &conn, FDRole role, uint32_t events)
    {
        uint32_t &prev_events = get_role_events(conn, role);

        if (prev_events == EPOLL_NOT_REGISTERED)
        {
            return;
        }

        if (prev_events != events)
        {
            prev_events = events;
            epoll_apply(conn, EPOLL_CTL_MOD, role, events);
        }
    }

    void remove_interest(Connections::connection_t &conn, FDRole role)
    {
        uint32_t &prev_events = get_role_events(conn, role);

        if (prev_events == EPOLL_NOT_REGISTERED)
        {
            return;
        }

        prev_events = EPOLL_NOT_REGISTERED;
        epoll_apply(conn, EPOLL_CTL_DEL, role, 0);
    }

    void cgi_handle_in(Connections::connection_t &conn);
    void cgi_handle_out(Connections::connection_t &conn);

    void sock_handle_write(Connections::connection_t &conn);
    void sock_handle_read(Connections::connection_t &conn);

    void update_events(Connections::connection_t &conn);
};

#endif
