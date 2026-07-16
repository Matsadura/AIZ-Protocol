#include "Response.hpp"
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstddef>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>

Response::Response(const s_Server &server, const Request &request) :
    m_status_code(200),
    m_request(request),
    // m_router(router),
    m_content_length(0),
    m_server(server)

{
    process();
}

Response::~Response(void)
{
}

/**
 * validate the request and prepare the response
 */
void Response::init_response()
{
    if (m_request.getState() != Request::COMPLETE)
    {
        m_status_code = m_request.getErrorCode();
        generateErrorBody();
        return;
    }
    std::string method = m_request.getMethod();
    std::string uri = m_request.getURI();
    Router router(m_server, uri, method);
    m_router = router.get_result();
    m_status_code = m_router.m_http_code;
    if (method == "GET")
    {
        init_GET();
    }
    // else if (method == "DELETE")
    // {
    //     init_DELETE();
    // }
    // else if (method == "POST")
    // {
    //     init_POST();
    // }
    // if (!m_body_content.empty())
    // {
    //     m_content_length = m_body_content.size();
    //     m_content_type   = get_content_type(file);
    // }
    // m_state = RESPONSE_SEND_HEADERS;
}

/**
 * header handler
 * @fd: socket fd
 */
void Response::header_builder()
{
    std::stringstream ss;
    ss << "HTTP/1.0 " << m_status_code << " " << getStatusMessage(m_status_code) << CRLF; // common
    if(!m_body_content.empty())
    {
        ss << "Content-Type: " << m_content_type << CRLF;
        ss << "Content-Length: " << m_body_content.size() << CRLF;
        ss << CRLF;
    }
    ss << m_body_content;
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
