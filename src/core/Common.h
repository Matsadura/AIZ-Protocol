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

// @note: This should use exceptions, std::abort will cause issue so change the name!
#define abort(msg)                                                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        std::cerr << __FILE__ << ":" << __LINE__ << ": " << std::endl;                                                 \
        perror(msg);                                                                                                   \
        exit(1);                                                                                                       \
    } while (0)

#define UNUSED(v) ((void)v)

#define UNIMPLEMENTED(message)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        std::cerr << __FILE__ << ':' << __LINE__ << ": UNIMPLEMENTED: " << message << "\n";                            \
        exit(1);                                                                                                       \
    } while (0)

#define UNREACHABLE(message)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        std::cerr << __FILE__ << ':' << __LINE__ << ": UNREACHABLE: " << message << "\n";                              \
        exit(1);                                                                                                       \
    } while (0)

#define TODO(message)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        std::cerr << __FILE__ << ':' << __LINE__ << ": TODO: " << message << "\n";                                     \
        exit(1);                                                                                                       \
    } while (0)

// Simple logger
// Usage: LOG_INFO("CONNECTION") << "new connection arrived"
#define LOG_INFO(module) (std::cout << "INFO: [" << (module) << "] > ")
#define LOG_DEBUG(module) (std::cout << "DEBUG: [" << (module) << "] > ")
#define LOG_ERROR(module) (std::cerr << "ERROR: [" << (module) << "] > ")

std::string addr_to_string(struct sockaddr_in *addr);
