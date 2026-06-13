#pragma once

#include "../Request/Request.hpp"
#include <fstream>
#include <cerrno>
#include <sys/stat.h>

#define NOT_FOUND 404
#define OK 200
#define FORBIDDEN 403
#define INTERNAL_SERVER_ERROR 500
class Response
{
  public:
    enum ResponseState
    {
        RESPONSE_INIT,
        RESPONSE_SEND_HEADERS,
        RESPONSE_SEND_CHUNKS,
        RESPONSE_COMPLETE,
        ERROR
    };

    Response(const Request &request);
    ~Response(void);

    void process(int fd);

  private:
    ResponseState m_state;
    size_t m_buffer_offset;
    int m_status_code;
    std::ifstream m_file_input;
    Request m_request;

    void   init_response();

};