#include "Multiplexer.h"
#include "Common.h"

Multiplexer::Multiplexer(void) : m_epfd(-1), m_evlist()
{
    // TODO: Read from config file and run all the listeners!
    m_listeners.create_new(NULL, "8080");

    struct epoll_event ev = {};
    m_epfd                = epoll_create(20);

    if (m_epfd == -1)
        abort("epoll_create");
    ev.events   = EPOLLIN;
    ev.data.u64 = pack_data(m_listeners[0], LISTENER);
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_listeners[0], &ev) == -1)
        abort("epoll_ctl");
}

Multiplexer::Multiplexer(const Multiplexer &other) // NOLINT
{
    UNUSED(other);
}

Multiplexer &Multiplexer::operator=(const Multiplexer &other) // NOLINT
{
    UNUSED(other);
    return (*this);
}

Multiplexer::~Multiplexer()
{
    if (m_epfd != -1)
        close(m_epfd);
}

inline int Multiplexer::unpack_conn_fd(uint64_t u64)
{
    return static_cast<int>(u64 >> 32);
}

Multiplexer::FDRole Multiplexer::unpack_role(uint64_t u64)
{
    return static_cast<FDRole>(u64 & 0xFFFFFFFF);
}

inline uint64_t Multiplexer::pack_data(int conn_fd, FDRole role)
{
    return (static_cast<uint64_t>(conn_fd) << 32) | static_cast<uint64_t>(role);
}

/**
 * Run the event loop to monitor and handle events on the listener sockets and active connections
 */
void Multiplexer::run()
{
    int ready;
    int i = 10;
    while (i-- > 0)
    {
        ready = epoll_wait(m_epfd, m_evlist, MAX_EVENTS, -1);
        for (int j = 0; j < ready; j++)
        {
            int fd      = unpack_conn_fd(m_evlist[j].data.u64);
            FDRole role = unpack_role(m_evlist[j].data.u64);
            log_event(m_evlist[j]);
            if (role == LISTENER)
            {
                // handle new connection
                m_conns.accept_new(fd, *this);
            }
            else if (role == CLIENT)
            {
                if (m_evlist[j].events & EPOLLIN)
                {
                    // Handle read
                    m_conns.conn_handle_read(fd, *this);
                }
                else if (m_evlist[j].events & EPOLLOUT)
                {
                    // Handle write
                    UNIMPLEMENTED("Handle epoll write event");
                }
                else if (m_evlist[j].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR))
                {
                    // Handle disconnection
                    m_conns.close_connection(fd, *this);
                }
            }
        }
    }
}

/**
 * Start monitoring the specified file descriptor for events
 */
void Multiplexer::start_monitor_conn(int conn_fd)
{
    struct epoll_event ev = {};
    ev.events             = EPOLLIN | EPOLLRDHUP;
    ev.data.u64           = pack_data(conn_fd, CLIENT);

    LOG_INFO("EPOLL") << "Registerd (fd=" << conn_fd << ")\n";
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, conn_fd, &ev) == -1)
        abort("epoll_ctl ADD");
}

/**
 * Change the settings associated with sockfd in the interest list to the new settings specified in events
 *
 * @sockfd: socket file descriptor
 * @evnts: which events to listen for EPOLLOUT | EPOLLIN
 */
void Multiplexer::switch_conn_interest(int conn_fd, uint32_t events)
{
    struct epoll_event ev = {};
    ev.events             = events;
    ev.data.u64           = pack_data(conn_fd, CLIENT);

    if (events == EPOLLIN)
        LOG_INFO("EPOLL") << "Mod->read (fd=" << conn_fd << ")\n";
    else if (events == EPOLLOUT)
        LOG_INFO("EPOLL") << "Mod->write (fd=" << conn_fd << ")\n";
    else
        UNREACHABLE("switch_interest got unxpected event");

    events = events | EPOLLRDHUP;
    if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, conn_fd, &ev) == -1)
        abort("epoll_ctl MOD");
}

/**
 * Stop monitoring the specified file descriptor for events
 */
void Multiplexer::stop_monitor_conn(int conn_fd)
{
    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, conn_fd, NULL) == 0)
        LOG_INFO("EPOLL") << "Remove (fd=" << conn_fd << ")\n";
    else
        abort("epoll_ctl");
}

/**
 * Log the details of the specified event for debugging and monitoring purposes
 */
void Multiplexer::log_event(struct epoll_event ev)
{
    LOG_INFO("EPOLL") << "New event: " << ((ev.events & EPOLLIN) ? "EPOLLIN " : "")
                      << ((ev.events & EPOLLOUT) ? "EPOLLOUT " : "") << ((ev.events & EPOLLHUP) ? "EPOLLHUP " : "")
                      << ((ev.events & EPOLLRDHUP) ? "EPOLLRDHUP " : "") << ((ev.events & EPOLLERR) ? "EPOLLERR " : "")
                      << "(fd=" << unpack_conn_fd(ev.data.u64) << ")\n";
}
