#include "ListenerAddrInfo.h"

ListenerAddrInfo::ListenerAddrInfo(const char *nodeName, const char *port) : m_hints(), m_result()
{
    m_hints.ai_family   = AF_INET;
    m_hints.ai_socktype = SOCK_STREAM;
    m_hints.ai_flags    = AI_PASSIVE;
    if (getaddrinfo(nodeName, port, &m_hints, &m_result) != 0)
        abort("getaddinfo");
}

ListenerAddrInfo::ListenerAddrInfo(const ListenerAddrInfo &other) // NOLINT
{
    (void)other;
}

ListenerAddrInfo &ListenerAddrInfo::operator=(const ListenerAddrInfo &other) // NOLINT
{
    (void)other;
    return *this;
}

std::string ListenerAddrInfo::toString()
{
    return addr_to_string(reinterpret_cast<struct sockaddr_in *>(m_result->ai_addr));
}

int ListenerAddrInfo::family()
{
    return m_result->ai_family;
}

int ListenerAddrInfo::sockType()
{
    return m_result->ai_socktype;
}

struct sockaddr *ListenerAddrInfo::addr()
{
    return m_result->ai_addr;
}

socklen_t ListenerAddrInfo::addr_len()
{
    return m_result->ai_addrlen;
}

ListenerAddrInfo::~ListenerAddrInfo()
{
    freeaddrinfo(m_result);
}
