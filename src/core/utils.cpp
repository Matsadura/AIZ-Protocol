#include "Common.h"

std::string addr_to_string(struct sockaddr_in *addr)
{
    std::stringstream ss;
    uint8_t *octets = reinterpret_cast<uint8_t *>(&addr->sin_addr.s_addr);

    ss << static_cast<unsigned int>(octets[0]) << ".";
    ss << static_cast<unsigned int>(octets[1]) << ".";
    ss << static_cast<unsigned int>(octets[2]) << ".";
    ss << static_cast<unsigned int>(octets[3]) << ":";

    ss << htons(addr->sin_port);

    return ss.str();
}
