#include "Multiplexer.h"

Multiplexer::Multiplexer(void) : m_epfd(), m_evlist()
{
    m_accepter.create_new(NULL, "8080"); //@hardcode: this suppose to be read from config file

    struct epoll_event ev = {};
    m_epfd                = epoll_create(20);

    if (m_epfd == -1)
        abort("epoll_create");
    ev.events  = EPOLLIN;
    ev.data.fd = m_accepter[0];
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_accepter[0], &ev) == -1)
        abort("epoll_ctl");
}

Multiplexer::Multiplexer(const Multiplexer &other) // NOLINT
{
    (void)other;
}

Multiplexer &Multiplexer::operator=(const Multiplexer &other) // NOLINT
{
    (void)other;
    return (*this);
}

Multiplexer::~Multiplexer()
{
}

/**
 * Run the event loop to monitor and handle events on the listener sockets and active connections
 */
void Multiplexer::run()
{ // NOLINT
    int ready;
    int i = 10;
    while (i-- > 0)
    {
        ready = epoll_wait(m_epfd, m_evlist, MAX_EVENTS, -1);
        for (int j = 0; j < ready; j++)
        {
            log_event(m_evlist[j]);
            if (m_accepter.contains(m_evlist[j].data.fd))
            {
                // handle new connection
                m_conns.accept_new(m_evlist[j].data.fd, *this);
            }
            else if (m_evlist[j].events & EPOLLRDHUP)
            {
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
    epoll_ctl(m_epfd, EPOLL_CTL_ADD, sockfd, &ev);
}

/**
 * Stop monitoring the specified file descriptor for events
 */
void Multiplexer::stop_monitoring(int sockfd)
{
    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, sockfd, NULL) == 0)
        std::cout << "INFO: [DISCONNECT] " << sockfd << " ignored by the poll\n";
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
    std::cout << ((ev.events & EPOLLHUP) ? "EPOLLHUP " : "");
    std::cout << ((ev.events & EPOLLERR) ? "EPOLLERR " : "");
    std::cout << "\n";
}
