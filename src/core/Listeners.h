#ifndef LISTENERS_H
#define LISTENERS_H

#include "../config_file_parser/parser/configfile.hpp"
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
    static int PandingLimit;
    // std::vector<int>        m_sockFds;
    std::map<int, s_Server> m_sockFds;

    /**
     * @note: this class is uncopyable
     */
    Listeners(const Listeners &other);
    Listeners &operator=(const Listeners &other);

  public:
    Listeners();
    ~Listeners();

    int         create_new(const s_Server &server);
    bool        contains(int fd);
    s_Server   *get_listener_config(int fd);
    std::size_t size();
};

#endif
