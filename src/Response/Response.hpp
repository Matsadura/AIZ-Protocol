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
    enum State
    {
        SEND_HEADER,
        SEND_BODY,
        COMPLETE
    };

    enum BodyType
    {
        BODY_NONE,
        BODY_STRING,
        BODY_FILE
    };

    Response(const Request &request, const RouterResult &router);
    ~Response(void);

    State getState() const
    {
        return m_state;
    }

  private:
    // params
    RouterResult m_router;
    Request      m_request;

    State    m_state;
    BodyType m_body_type;

    // HTTP metadata
    int         m_status_code;
    std::string m_content_type;
    size_t      m_content_length;
    std::string m_location;
    //  Header
    std::vector<char> m_header;
    size_t            m_header_size;
    // String body
    std::vector<char> m_body;
    // File body
    std::ifstream m_file;
    // response buffer
    std::vector<char> m_response_buffer;
    size_t            m_buffer_offset;

    const std::vector<char> &getResponseBuffer();
    bool                     isFinished() const;
    void                     consume(size_t written);

  private:
    void        buildHeader();
    void        read_file();
    std::string getStatusMessage(int code);
    void        generateErrorBody();

    std::string get_content_type(const std::string &filepath);
    void        prepare_response();
    void        FillBuffer();
};

size_t get_file_length(std::string &file);