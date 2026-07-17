#pragma once

#include "../Request/Request.hpp"
#include "../Router/Router.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
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
#define HTTP_MOVED_PERMANENTLY 301
#define HTTP_FOUND 302
#define HTTP_SEE_OTHER 303
#define HTTP_TEMPORARY_REDIRECT 307
#define HTTP_PERMANENT_REDIRECT 308
#define BUFFER_SIZE 64000

class Response
{
  public:
    enum ResponseState
    {
        SEND_HEADER,
        SEND_CHUNKS,
        COMPLETE
    };

    Response(const s_Server &server, const Request &request, const RouterResult &router);
    ~Response(void);

    void get_response();

    // Getters
    const std::string &getResponseBuffer();

    ResponseState getState() const
    {
        return m_state;
    }

  private:
    int               m_status_code;
    ResponseState     m_state;
    std::ifstream     m_file_input;
    Request           m_request;
    RouterResult      m_router;
    std::vector<char> m_response_buffer[64000];
    std::vector<char> m_body_content;
    size_t            m_content_length;
    std::string       m_content_type;
    s_Server          m_server;
    size_t            m_header_size;
    std::ifstream     m_infile;
    size_t            m_offset;
    
    void        Body_builder();
    void        handle_error(int fd);
    void        header_builder();
    std::string getStatusMessage(int code);
    void        chunks_handler();
    std::string toHex(size_t size);
    void        generateErrorBody();
    void        init_GET();
    void        init_DELETE();
    void        init_POST();
    std::string get_content_type(const std::string &filepath);
  std::vector<char> get_response(int written);

};