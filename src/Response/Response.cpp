#include "Response.hpp"
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstddef>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>

Response::Response(const s_Server &server, const Request &request, const RouterResult &router) :
    m_status_code(200),
    m_state(SEND_HEADER),
    m_request(request),
    m_router(router),
    m_content_length(0),
    m_server(server),
    m_header_size(0),
    m_offset(0)

{
    get_response();
}

Response::~Response(void)
{
}

/**
 * validate the request and prepare the response
 */
void Response::Body_builder()
{
    char *buffer;
    if (m_router.m_data_type == RouterResult::FILE_PATH)
    {
        m_infile.open(m_router.m_data.c_str(), std::ios::binary);
        if (!m_infile.is_open())
        {
            generateErrorBody();
            return;
        }
        m_infile.read(buffer, BUFFER_SIZE - m_header_size);
        m_response_buffer->insert(m_response_buffer->end(), buffer, buffer + std::strlen(buffer));
    }
    else if (m_router.m_data_type == RouterResult::STRING_BUFFER)
    {
        m_response_buffer->insert(m_response_buffer->end(), m_router.m_data.begin(), m_router.m_data.end());
    }
    m_content_length = m_body_content.size();
    m_state          = SEND_CHUNKS;
}

size_t get_file_length(std::string &file)
{
    struct stat st = {};
    if (stat(file.c_str(), &st) == 0)
        return st.st_size;
    return 0;
}

/**
 * header handler
 * @fd: socket fd
 */
void Response::header_builder()
{
    std::stringstream ss;
    ss << "HTTP/1.0 " << m_status_code << " " << getStatusMessage(m_status_code) << CRLF; // common

    if (m_router.m_data_type == RouterResult::REDIRECTION)
    {
        ss << "Location: " << m_router.m_data << CRLF;
    }
    ss << "Content-Type: " << m_content_type << CRLF;
    ss << "Content-Length: " << m_content_length << CRLF;
    ss << CRLF;
    std::string str = ss.str();
    m_response_buffer->insert(m_response_buffer->end(), str.begin(), str.end());
    m_header_size = str.size();
    m_state       = SEND_CHUNKS;
}

// std::string Response::toHex(size_t size)
// {
//     std::stringstream ss;

//     ss << std::hex << size;
//     return ss.str();
// }
void Response::fill_buffer()
{
    m_response_buffer->erase(m_response_buffer->begin(), m_response_buffer->begin() + m_offset);
    if(m_response_buffer->empty())
        
}

std::vector<char> Response::get_response(size_t written)
{
    if (m_status_code == SEND_HEADER)
    {
        m_status_code  = m_router.m_http_code;
        m_content_type = get_content_type(m_router.m_data);
        if (m_router.m_data_type == RouterResult::FILE_PATH)
            m_content_length = get_file_length(m_router.m_data);
        else if (m_router.m_data_type == RouterResult::STRING_BUFFER)
            m_content_length = m_router.m_data.length();
        else
            m_content_length = 0;
        header_builder();
        Body_builder(); // fills the body content
    }
    else if (m_status_code == SEND_CHUNKS)
    {
        m_offset = written;
        fill_buffer();
    }
    else
    {
    }
}
