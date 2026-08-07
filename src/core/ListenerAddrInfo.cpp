#include "ListenerAddrInfo.h"
#include <cassert>
#include <stdexcept>

static std::string create_report_messsage(const char *function_name, const std::string &node,
                                          const std::string &service, int error_code)
{
    // NOTE: Error prefix and last new line will be added in the place where this error will be caught
    // Furor: Failed to prepare listener socket for '0.0.0.0:8080'.
    // Cause: getaddrinfo() failed (Address already in use).
    std::ostringstream message;
    message << "Failed to prepare address info for '" << node << ":" << service << "'.\n";
    message << "    Cause: " << function_name << "() failed (" << gai_strerror(error_code) << ").";
    return message.str();
}

/**
 * This is basically RAII wrapper for addrinfo
 *
 * @nodeName: hostname or numeric address or NULL to get the address info for all interfaces
 * @port: The port number to get the address info for
 */
ListenerAddrInfo::ListenerAddrInfo(const char *nodeName, const char *port) : m_hints(), m_result()
{
    m_hints.ai_family   = AF_INET;
    m_hints.ai_socktype = SOCK_STREAM;
    m_hints.ai_flags    = AI_PASSIVE;
    int error_code      = getaddrinfo(nodeName, port, &m_hints, &m_result);
    if (error_code != 0)
    {
        throw std::runtime_error(create_report_messsage("getaddrinfo", nodeName, port, error_code));
    }
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

/**
 * Get the string representation of the address info
 */
std::string ListenerAddrInfo::toString()
{
    return addr_to_string(reinterpret_cast<const struct sockaddr_storage *>(m_result->ai_addr));
}

/**
 * Get type of the socket address structure
 */
int ListenerAddrInfo::family()
{
    return m_result->ai_family;
}

/**
 * Indicating whether address structure is for a TCP or a UDP service
 */
int ListenerAddrInfo::sockType()
{
    return m_result->ai_socktype;
}

/**
 * Get underlaying socket address structure, where port and address e.g. IPv4 contains
 */
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
