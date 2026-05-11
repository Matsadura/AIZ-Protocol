#include "Multiplexer.h"
#include "Common.h"

Multiplexer::Multiplexer(void) : m_epfd(-1), m_evlist()
{
    m_listeners.create_new(NULL, "8080"); // @hardcode: this suppose to be read from config file

    struct epoll_event ev = {};
    m_epfd                = epoll_create(20);

    if (m_epfd == -1)
        abort("epoll_create");
    ev.events  = EPOLLIN;
    ev.data.fd = m_listeners[0];
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
            log_event(m_evlist[j]);
            if (m_listeners.contains(m_evlist[j].data.fd))
            {
                // handle new connection
                m_conns.accept_new(m_evlist[j].data.fd, *this);
            }
            else if (m_evlist[j].events & EPOLLIN)
            {
                // Handle read
                m_conns.handle_read(m_evlist[j].data.fd, *this);
            }
            else if (m_evlist[j].events & EPOLLOUT)
            {
                // Handle write
                UNIMPLEMENTED("Handle epoll write event");
            }
            else if (m_evlist[j].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR))
            {
                // Handle disconnection
                m_conns.close_connection(m_evlist[j].data.fd, *this);
            }
        }
    }
}

/**
 * Start monitoring the specified file descriptor for events
 */
void Multiplexer::start_monitoring(int sockfd)
{
    struct epoll_event ev = {};
    ev.events             = EPOLLIN | EPOLLRDHUP;
    ev.data.fd            = sockfd;

    std::cout << "INFO: [NEW_CONNECTION] (fd=" << sockfd << ") registerd in interest list\n";
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, sockfd, &ev) == -1)
        abort("epoll_ctl ADD");
}

void Multiplexer::switch_interest(int sockfd, uint32_t events)
{
    struct epoll_event ev = {};
    ev.events             = events;
    ev.data.fd            = sockfd;

    if (events == EPOLLIN)
        std::cout << "INFO: [SWITCH_INTEREST] (fd=" << sockfd << ") wants to read\n";
    else if (events == EPOLLOUT)
        std::cout << "INFO: [SWITCH_INTEREST] (fd=" << sockfd << ") wants to respond\n";
    else
        UNREACHABLE("switch_interest got unxpected event");

    events = events | EPOLLRDHUP;
    if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, sockfd, &ev) == -1)
        abort("epoll_ctl MOD");
}

/**
 * Stop monitoring the specified file descriptor for events
 */
void Multiplexer::stop_monitoring(int sockfd)
{
    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, sockfd, NULL) == 0)
        std::cout << "INFO: [DISCONNECT] " << sockfd << " deregisterd from interest list\n";
    else
        abort("epoll_ctl");
}

/**
 * Log the details of the specified event for debugging and monitoring purposes
 */
void Multiplexer::log_event(struct epoll_event ev)
{
    std::cout << "INFO: " << "[EVENT] (fd=" << ev.data.fd << ") " << "TYPE: ";
    std::cout << ((ev.events & EPOLLIN) ? "EPOLLIN " : "");
    std::cout << ((ev.events & EPOLLOUT) ? "EPOLLOUT " : "");
    std::cout << ((ev.events & EPOLLHUP) ? "EPOLLHUP " : "");
    std::cout << ((ev.events & EPOLLRDHUP) ? "EPOLLRDHUP " : "");
    std::cout << ((ev.events & EPOLLERR) ? "EPOLLERR " : "");
    std::cout << "\n";
}
