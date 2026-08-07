#include "Common.h"

std::string get_addr_port_string(const struct sockaddr_storage *addr)
{
    std::stringstream ss;

    if (addr->ss_family == AF_INET)
    {
        const struct sockaddr_in *addr4 = reinterpret_cast<const struct sockaddr_in *>(addr);
        ss << ntohs(addr4->sin_port);
    }
    else
    {
        ss << "XX";
    }

    return ss.str();
}

std::string get_addr_host_string(const struct sockaddr_storage *addr)
{
    std::stringstream ss;

    if (addr->ss_family == AF_INET)
    {
        const struct sockaddr_in *addr4  = reinterpret_cast<const struct sockaddr_in *>(addr);
        const uint8_t            *octets = reinterpret_cast<const uint8_t *>(&addr4->sin_addr.s_addr);

        ss << static_cast<unsigned int>(octets[0]) << ".";
        ss << static_cast<unsigned int>(octets[1]) << ".";
        ss << static_cast<unsigned int>(octets[2]) << ".";
        ss << static_cast<unsigned int>(octets[3]);
    }
    else
    {
        ss << "X.X.X.X";
    }

    return ss.str();
}

/**
 * Convert a sockaddr_in structure to IPv4 string representation
 */
std::string addr_to_string(const struct sockaddr_storage *addr)
{
    return get_addr_host_string(addr) + ":" + get_addr_port_string(addr);
}
