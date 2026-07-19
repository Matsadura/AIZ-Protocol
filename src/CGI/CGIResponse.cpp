#include "CGIResponse.hpp"

CGIResponse::CGIResponse() : m_cgi_state(CGI_IDLE), m_is_local_redirect(false)
{
}

CGIResponse::~CGIResponse()
{
}

void CGIResponse::setCgiState(CgiState state)
{
    m_cgi_state = state;
}

CGIResponse::CgiState CGIResponse::getCgiState() const
{
    return m_cgi_state;
}

void CGIResponse::appendCgiData(const char *data, size_t length)
{
    if (m_cgi_state == CGI_READING_HEADERS)
    {
        m_cgi_header_buffer.append(data, length);
        if (parseCgiHeaders())
            m_cgi_state = CGI_STREAMING_BODY;
    }
    else if (m_cgi_state == CGI_STREAMING_BODY)
        m_body_buffer.insert(m_body_buffer.end(), data, data + length);
}

bool CGIResponse::parseCgiHeaders()
{
    size_t boundary_pos = m_cgi_header_buffer.find("\r\n\r\n");
    size_t boundary_len = 4;

    if (boundary_pos == std::string::npos)
    {
        boundary_pos = m_cgi_header_buffer.find("\n\n");
        boundary_len = 2;
    }

    if (boundary_pos == std::string::npos)
        return false;

    std::string headers_part = m_cgi_header_buffer.substr(0, boundary_pos);
    std::string leftover_body = m_cgi_header_buffer.substr(boundary_pos + boundary_len);

    m_body_buffer.insert(m_body_buffer.end(), leftover_body.begin(), leftover_body.end());

    // 1 - TODO: PARSE HEADERS

    // 2 - TRANSLATE TO HTTP HEADERS
    return true;
}