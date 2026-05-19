#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include "Common.h"
#include "Connections.h"
#include "Listeners.h"

#define MAX_EVENTS 1024

class Multiplexer
{
  private:
    int m_epfd;
    struct epoll_event m_evlist[MAX_EVENTS];
    Connections m_conns;
    Listeners m_listeners;

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
    void log_event(struct epoll_event event);
    void start_monitor_conn(int conn_fd);
    void stop_monitor_conn(int conn_fd);
    void switch_conn_interest(int conn_fd, uint32_t events);
};

#endif
