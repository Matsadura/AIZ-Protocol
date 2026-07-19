#include "CGIResponse.hpp"
#include <sstream>

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

void CGIResponse::translateToHttp(std::map<std::string, std::string> &cgi_headers)
{
    std::ostringstream http_headers;
    std::string        status = "200 OK";

    if (cgi_headers.count("status"))
        status = cgi_headers["status"];
    else if (cgi_headers.count("location"))
    {
        status              = "302 Found";
        m_is_local_redirect = true;
    }

    http_headers << "HTTP/1.1 " << status << "\r\n";

    if (cgi_headers.count("content-type"))
        http_headers << "Content-Type: " << cgi_headers["content-type"] << "\r\n";
    else
        http_headers << "Content-Type: text/html\r\n"; // Default content type (Change it? Removie it? idk)

    http_headers << "Transfer-encoding: chunked\r\n";

    for (std::map<std::string, std::string>::const_iterator it = cgi_headers.begin(); it != cgi_headers.end(); ++it)
    {
        const std::string &key   = it->first;
        const std::string &value = it->second;

        if (key != "status" && key != "content-type" && key != "location")
            http_headers << key << ": " << value << "\r\n";
    }

    http_headers << "\r\n";
    m_http_response_headers = http_headers.str();
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

    std::string headers_part  = m_cgi_header_buffer.substr(0, boundary_pos);
    std::string leftover_body = m_cgi_header_buffer.substr(boundary_pos + boundary_len);

    m_body_buffer.insert(m_body_buffer.end(), leftover_body.begin(), leftover_body.end());

    std::vector<std::string> header_lines = split(headers_part, '\n');

    std::map<std::string, std::string> parsed_cgi_headers;

    // 1 - PARSE HEADERS, TO_DO -> Validate headers
    for (std::vector<std::string>::const_iterator it = header_lines.begin(); it != header_lines.end(); ++it)
    {
        const std::string &line      = *it;
        size_t             colon_pos = line.find(':');
        if (colon_pos != std::string::npos)
        {
            std::string key   = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            trim(key);
            trim(value);
            // TODO -> Validate headers
            parsed_cgi_headers[key] = value;
        }
    }

    // 2 - TRANSLATE TO HTTP HEADERS
    translateToHttp(parsed_cgi_headers);
    return true;
}