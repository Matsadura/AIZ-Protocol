#include "Response.hpp"
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>

Response::Response(const RouterResult &router) :
    m_router(router),
    m_state(STREAMING),
    m_body_type(BODY_NONE),
    m_status_code(m_router.m_http_code),
    m_content_length(0),
    m_header_size(0),
    m_buffer_offset(0)

{
    prepare_response();
    buildHeader();
    FillBuffer();
}

Response::~Response(void)
{
}

void Response::buildHeader()
{
    std::stringstream ss;
    ss << "HTTP/1.0" << " " << m_status_code << CRLF;
    if (!m_location.empty())
        ss << "Location: " << m_location << CRLF;
    ss << "Content-Type: " << m_content_type << CRLF;
    ss << "Content-Length: " << m_content_length << CRLF;
    ss << CRLF;
    std::string str = ss.str();
    m_header.assign(str.begin(), str.end());
    m_header_size = m_header.size();
}

void Response::generateErrorBody()
{
    std::stringstream ss;

    ss << "<!DOCTYPE html>\n";
    ss << "<html>\n";
    ss << "<head><title>";
    ss << m_status_code << " " << getStatusMessage(m_status_code);
    ss << "</title></head>\n";

    ss << "<body>\n";
    ss << "<h1>";
    ss << m_status_code << " " << getStatusMessage(m_status_code);
    ss << "</h1>\n";

    ss << "<hr>\n";
    ss << "<p>Webserv</p>\n";
    ss << "</body>\n";
    ss << "</html>";

    std::string html = ss.str();

    m_body.assign(html.begin(), html.end());

    m_body_type      = BODY_STRING;
    m_content_type   = "text/html";
    m_content_length = m_body.size();
}

void Response::prepare_response()
{
    if (m_router.m_data_type == RouterResult::FILE_PATH)
    {
        if (!m_router.m_data.empty())
        {
            m_body_type      = BODY_FILE;
            m_content_length = get_file_length(m_router.m_data);
            m_content_type   = get_content_type(m_router.m_data);
            m_file.open(m_router.m_data.c_str(), std::ios::binary);
            if (!m_file.is_open())
            {
                m_status_code = 500;
                generateErrorBody();
            }
        }
        else
        {
            generateErrorBody();
        }
    }
    else if (m_router.m_data_type == RouterResult::STRING_BUFFER)
    {
        m_body_type         = BODY_STRING;
        std::string content = m_router.m_data;
        m_content_type      = "text/html";
        m_content_length    = content.length();
        m_body.assign(m_router.m_data.begin(), m_router.m_data.end());
    }
    else if (m_router.m_data_type == RouterResult::REDIRECTION)
    {
        m_body_type      = BODY_NONE;
        m_location       = m_router.m_data;
        m_content_length = 0;
    }
    else
    {
        std::cout << "Something inexpected happened in prepare response\n";
    }
}

void Response::FillBuffer()
{
    m_response_buffer.assign(m_header.begin(), m_header.end());
    if (m_body_type == BODY_STRING)
    {
        m_response_buffer.insert(m_response_buffer.end(), m_body.begin(), m_body.end());
    }
    else if (m_body_type == BODY_FILE)
    {
        read_file();
    }
}

void Response::read_file()
{
    char   buff[BUFFER_SIZE];
    size_t remaining = BUFFER_SIZE - m_response_buffer.size();
    m_file.read(buff, static_cast<std::streamsize>(remaining));
    size_t read = static_cast<size_t>(m_file.gcount());
    if (!read)
    {
        m_state = COMPLETE;
        m_file.close();
    }
    else
        m_response_buffer.insert(m_response_buffer.end(), buff, buff + m_file.gcount());
}

const std::vector<char> &Response::getResponseBuffer()
{
    return m_response_buffer;
}

void Response::consume(size_t written)
{
    if (m_state == COMPLETE)
        return;
    written = std::min(written, m_response_buffer.size());
    m_response_buffer.erase(m_response_buffer.begin(), m_response_buffer.begin() + static_cast<int>(written));
    if (m_response_buffer.empty())
    {
        if (m_body_type == BODY_STRING || m_body_type == BODY_NONE)
            m_state = COMPLETE;
        else
            read_file();
    }
}

bool Response::isFinished() const
{
    return (m_state == COMPLETE);
}