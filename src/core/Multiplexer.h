#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include "Common.h"
#include "Connections.h"
#include "Listeners.h"

#define MAX_EVENTS 1024

class Multiplexer : public Uncopyable
{
  private:
    int m_epfd;
    struct epoll_event m_evlist[MAX_EVENTS];
    Connections m_conns;
    Listeners m_accepter;

  public:
    Multiplexer(void);
    Multiplexer(const Multiplexer &other);
    Multiplexer &operator=(const Multiplexer &other);
    ~Multiplexer();

    void run();
    void log_event(struct epoll_event event);
    void start_monitoring(int sockfd);
    void stop_monitoring(int sockfd);
};

#endif
