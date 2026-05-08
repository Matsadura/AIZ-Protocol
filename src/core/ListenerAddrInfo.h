#ifndef LISTENER_ADDR_INFO_H
#define LISTENER_ADDR_INFO_H

#include "Common.h"

class ListenerAddrInfo : public Uncopyable
{
  private:
    struct addrinfo m_hints;
    struct addrinfo *m_result;

  public:
    ListenerAddrInfo(const char *nodeName, const char *port);
    ListenerAddrInfo(const ListenerAddrInfo &other);
    ListenerAddrInfo &operator=(const ListenerAddrInfo &other);
    ~ListenerAddrInfo();
    std::string toString();
    int family();
    int sockType();
    struct sockaddr *addr();
    socklen_t addr_len();
};

#endif
