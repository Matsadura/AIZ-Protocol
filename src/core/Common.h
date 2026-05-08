#pragma once

// C++ Standard Library
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

// C Standard Library
#include <cstdio>
#include <cstdlib>

// POSIX & Networking APIs
#include <netdb.h>      // Host/network resolution (getaddrinfo, addrinfo)
#include <sys/epoll.h>
#include <sys/socket.h> // Socket primitives (socket, bind, listen, accept)
#include <unistd.h>

#define abort(msg)                                                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        std::cerr << __FILE__ << ":" << __LINE__ << ": " << std::endl;                                                 \
        perror(msg);                                                                                                   \
        exit(1);                                                                                                       \
    } while (0)

/**
 * Base class designed to prevent copying
 **/
class Uncopyable
{
  protected:
    Uncopyable()
    {
    }

    ~Uncopyable()
    {
    }

  private:
    Uncopyable(const Uncopyable &);
    Uncopyable &operator=(const Uncopyable &);
};

std::string addr_to_string(struct sockaddr_in *addr);
