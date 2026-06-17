#pragma once

#include "../Request/Request.hpp"
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sys/stat.h>
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
        RESPONSE_COMPLETE,
    };

    Response(const Request &request);
    ~Response(void);
    
    void process();
    
    // Getters
    const std::string &getResponseBuffer();
    ResponseState getState() const { return m_state; }

  private:
    ResponseState m_state;
    size_t m_buffer_offset;
    int m_status_code;
    std::ifstream m_file_input;
    Request m_request;
    std::string m_response_buffer;
    std::string m_body_content;
    size_t m_content_length;
    std::string m_content_type;

    void init_response();
    void handle_error(int fd);
    void header_builder();
    std::string getStatusMessage(int code);
    void chunks_handler();
    std::string toHex(size_t size);
    void generateErrorBody();
    void init_GET(const std::string &file_path);
    void init_DELETE(const std::string &file_path);
    void init_POST(const std::string &file_path);
    std::string get_content_type(const std::string &filepath);


};