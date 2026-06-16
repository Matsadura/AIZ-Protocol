#pragma once

#include "../Request/Request.hpp"
#include <fstream>
#include <cerrno>
#include <string>
#include <sys/stat.h>
#include <cstdlib>
#include <unistd.h>

#define NOT_FOUND 404
#define OK 200
#define FORBIDDEN 403
#define INTERNAL_SERVER_ERROR 500
#define CHUNK_BUFFER_SIZE 4096
#define NO_CONTENT 204
#define CREATED 201
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

    void process();

  private:
    ResponseState m_state;
    size_t m_buffer_offset;
    int m_status_code;
    std::ifstream m_file_input;
    Request m_request;
    std::string m_response_buffer;
    std::string m_body_content;
    size_t m_content_length;

    void   init_response();
    void   handle_error(int fd);
    void   header_handler();
    std::string getStatusMessage(int code);
    void    chunks_handler();
    std::string toHex(size_t size);
    void    handleErrors();
  


};