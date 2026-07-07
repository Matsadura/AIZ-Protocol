#include "Response.hpp"
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstddef>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>

Response::Response(const Request &request) :
    m_state(RESPONSE_INIT), m_buffer_offset(0), m_status_code(200), m_request(request), m_content_length(0)

{
}

Response::~Response(void)
{
}

/**
 * validate the request and prepare the response
 */
void Response::init_response()
{
    // if (m_request.getState() != Request::COMPLETE)
    // {
        // m_status_code = m_request.getErrorCode();
        // generateErrorBody();
        // return;
    // }
    std::string file   = "inddex.html"; // index.html to be replaced by the actual path provided by the router
    std::string method = m_request.getMethod();

    if (method == "GET")
    {
        init_GET(file);
    }
    else if (method == "DELETE")
    {
        init_DELETE(file);
    }
    else if (method == "POST")
    {
        init_POST(file);
    }
    if (!m_body_content.empty())
    {
        m_content_length = m_body_content.size();
        m_content_type   = get_content_type(file);
    }
    m_state = RESPONSE_SEND_HEADERS;
}

/**
 * header handler
 * @fd: socket fd
 */
void Response::header_builder()
{
    std::stringstream ss;
    // std::string filepath = "index.html/";
    ss << "HTTP/1.0 " << m_status_code << " " << getStatusMessage(m_status_code) << CRLF; // common
    // ss << "Server: webserv/1.0" << CRLF;
    if (m_status_code != 204)     // if not DELETE success
    {
        if (m_status_code == 200) // if GET success
            ss << "Content-Type: " << m_content_type << CRLF;
        else                      // if POST success or any error
            ss << "Content-Type: text/html" << CRLF;
        ss << "Content-Length: " << m_content_length << CRLF;
    }
    ss << CRLF;

    m_response_buffer = ss.str() + m_body_content;
    m_state           = RESPONSE_COMPLETE;
}

// std::string Response::toHex(size_t size)
// {
//     std::stringstream ss;

//     ss << std::hex << size;
//     return ss.str();
// }

void Response::process()
{
    init_response();
    header_builder();
}
