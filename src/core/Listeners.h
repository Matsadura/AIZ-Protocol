#ifndef LISTENERS_H
#define LISTENERS_H

#include "Common.h"
#include "ListenerAddrInfo.h"

/**
 * Manage multiple listener sockets and their file descriptors
 */
class Listeners
{
  private:
    /**
     * The kernel must record some information about each pending connection request
     * so that a subsequent accept() can be processed. The backlog argument allows us to
     * limit the number of such pending connections. Connection requests up to this limit
     * succeed immediately. Further connection requests block until a pending connection
     * is accepted (via accept()), and thus removed from the queue of pending connections.
     */
    static int       PandingLimit;
    std::vector<int> m_sockFds;

    /**
     * @note: this class is uncopyable
     */
    Listeners(const Listeners &other);
    Listeners &operator=(const Listeners &other);

  public:
    Listeners();
    ~Listeners();

    void        create_new(const char *nodeName, const char *port);
    bool        contains(int fd);
    std::size_t size();
    int         operator[](std::size_t index);
};

#endif
