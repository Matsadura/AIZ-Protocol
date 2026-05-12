#ifndef LISTENER_ADDR_INFO_H
#define LISTENER_ADDR_INFO_H

#include "Common.h"

class ListenerAddrInfo
{
  private:
    struct addrinfo m_hints;
    struct addrinfo *m_result;

    /**
     * @note: this class is uncopyable
     */
    ListenerAddrInfo(const ListenerAddrInfo &other);
    ListenerAddrInfo &operator=(const ListenerAddrInfo &other);

  public:
    ListenerAddrInfo(const char *nodeName, const char *port);
    ~ListenerAddrInfo();

    std::string toString();
    int family();
    int sockType();
    struct sockaddr *addr();
    socklen_t addr_len();
};

#endif
