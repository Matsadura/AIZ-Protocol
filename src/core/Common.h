#pragma once

// C++ Standard Library
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

// C Standard Library
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// POSIX & Networking APIs
#include <netdb.h>      // Host/network resolution (getaddrinfo, addrinfo)
#include <sys/epoll.h>
#include <sys/socket.h> // Socket primitives (socket, bind, listen, accept)
#include <unistd.h>

/**
 * Log error context and exit the application
 *
 * @msg The error context to pass to perror
 */
// @note: This should use exceptions, std::abort will cause issue so change the name!
#define abort(msg)                                                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        std::cerr << __FILE__ << ":" << __LINE__ << ": ";                                                              \
        perror(msg);                                                                                                   \
        exit(1);                                                                                                       \
    } while (0)

/**
 * Suppress unused variable compiler warnings
 *
 * @v The variable to mark as intentionally unused
 */
#define UNUSED(v) ((void)v)

/**
 * Terminate execution when hitting a missing feature
 *
 * @message Details about what functionality is missing
 */
#define UNIMPLEMENTED(message)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        std::cerr << __FILE__ << ':' << __LINE__ << ": UNIMPLEMENTED: " << message << "\n";                            \
        exit(1);                                                                                                       \
    } while (0)

/**
 * Terminate execution on supposedly impossible code paths
 *
 * @message Context explaining why this state is logically unreachable
 */
#define UNREACHABLE(message)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        std::cerr << __FILE__ << ':' << __LINE__ << ": UNREACHABLE: " << message << "\n";                              \
        exit(1);                                                                                                       \
    } while (0)

/**
 * Terminate execution to highlight pending development work
 *
 * @message Description of the task left to be done
 */
#define TODO(message)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        std::cerr << __FILE__ << ':' << __LINE__ << ": TODO: " << message << "\n";                                     \
        exit(1);                                                                                                       \
    } while (0)

/**
 * Stream-based logging macros
 * Usage: LOG_INFO("MODULE") << "message\n";
 *
 * @module String literal identifying the originating subsystem
 */
#define LOG_INFO(module) (std::cout << "INFO: [" << (module) << "] > ")
#define LOG_DEBUG(module) (std::cout << "DEBUG: [" << (module) << "] > ")
#define LOG_ERROR(module) (std::cerr << "ERROR: [" << (module) << "] > ")

std::string addr_to_string(struct sockaddr_in *addr);
