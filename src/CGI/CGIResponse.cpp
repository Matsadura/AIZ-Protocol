#include "CGIResponse.hpp"
#include <cstddef>
#include <sstream>

CGIResponse::CGIResponse() :
    m_cgi_state(CGI_IDLE),
    m_is_local_redirect(false),
    m_error_code(0),
    m_already_send_count(0),
    m_is_chunked_response(true)
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

bool CGIResponse::isLocalRedirect() const
{
    return m_is_local_redirect;
}

int CGIResponse::getErrorCode() const
{
    return m_error_code;
}

std::vector<char> &CGIResponse::getBodyBuffer()
{
    return m_body_buffer;
}

bool CGIResponse::isBufferFull() const
{
    return m_body_buffer.size() >= CHUNK_SIZE_LIMIT;
}

std::size_t CGIResponse::getAlreadySendCount() const
{
    return m_already_send_count;
}

/**
 * appendCgiData - Appends CGI data and formats it immediately into HTTP chunks.
 * @data: A pointer to the data to be appended.
 * @length: The length of the data to be appended.
 */
void CGIResponse::appendCgiData(const char *data, size_t length)
{
    if (m_error_code != 0)
        return;

    if (m_cgi_state == CGI_IDLE)
        m_cgi_state = CGI_READING_HEADERS;

    if (m_cgi_state == CGI_READING_HEADERS)
    {
        m_cgi_header_buffer.append(data, length);
        if (parseCgiHeaders())
        {
            if (m_error_code == 0)
                m_cgi_state = CGI_STREAMING_BODY;
        }
    }
    else if (m_cgi_state == CGI_STREAMING_BODY)
    {
        if (length > 0)
        {
            if (m_is_chunked_response)
            {
                std::ostringstream hex_size;
                hex_size << std::hex << length << "\r\n";
                std::string chunk_hdr = hex_size.str();

                m_body_buffer.insert(m_body_buffer.end(), chunk_hdr.begin(), chunk_hdr.end());
                m_body_buffer.insert(m_body_buffer.end(), data, data + length);
                m_body_buffer.push_back('\r');
                m_body_buffer.push_back('\n');
            }
            else
                m_body_buffer.insert(m_body_buffer.end(), data, data + length);
        }
    }
}

/**
 * translateToHttp - Translates CGI headers to an HTTP response string.
 * @cgi_headers: A map containing CGI headers to be translated.
 */
std::string CGIResponse::translateToHttp(std::map<std::string, std::string> &cgi_headers)
{
    std::ostringstream http_headers;
    std::string        status = "200 OK";
    std::string        location_line;

    if (cgi_headers.count("status"))
        status = cgi_headers["status"];
    if (cgi_headers.count("location"))
    {
        std::string loc = cgi_headers["location"];
        if (!loc.empty() && loc[0] == '/')
            m_is_local_redirect = true;
        else
        {
            status = "302 Found";
            location_line += "Location: " + loc + "\r\n";
        }
    }

    http_headers << "HTTP/1.1 " << status << "\r\n";
    http_headers << location_line;

    if (cgi_headers.count("content-type"))
        http_headers << "Content-Type: " << cgi_headers["content-type"] << "\r\n";

    if (cgi_headers.count("content-length"))
    {
        http_headers << "Content-Length: " << cgi_headers["content-length"] << "\r\n";
        m_is_chunked_response = false;
    }
    else
    {
        http_headers << "Transfer-encoding: chunked\r\n";
        m_is_chunked_response = true;
    }

    for (std::map<std::string, std::string>::const_iterator it = cgi_headers.begin(); it != cgi_headers.end(); ++it)
    {
        const std::string &key   = it->first;
        const std::string &value = it->second;

        if (key != "status" && key != "content-type" && key != "location" && key != "content-length")
            http_headers << key << ": " << value << "\r\n";
    }

    http_headers << "\r\n";
    return http_headers.str();
}

/**
 * parseCgiHeaders - Parses the CGI headers and injects HTTP framing into the output buffer.
 */
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

    std::vector<std::string>           header_lines = split(headers_part, '\n');
    std::map<std::string, std::string> parsed_cgi_headers;

    for (std::vector<std::string>::const_iterator it = header_lines.begin(); it != header_lines.end(); ++it)
    {
        std::string line = *it;

        while (!line.empty() && (line[line.length() - 1] == '\r' || line[line.length() - 1] == '\n'))
        {
            line.erase(line.length() - 1);
        }

        if (line.empty())
            continue;

        if (line[0] == ' ' || line[0] == '\t')
        {
            generateErrorResponse(502);
            return true;
        }

        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos)
        {
            generateErrorResponse(502);
            return true;
        }

        std::string key   = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);

        if (!key.empty() && (key[key.length() - 1] == ' ' || key[key.length() - 1] == '\t'))
        {
            generateErrorResponse(502);
            return true;
        }

        key   = trim(key);
        value = trim(value);
        key   = toLower(key);

        if (!isValidHeaderName(key) || !isValidHeaderValue(value))
        {
            generateErrorResponse(502);
            return true;
        }

        if (value.empty())
            continue;

        if (key == "content-type" || key == "location" || key == "status")
        {
            if (parsed_cgi_headers.count(key) > 0)
            {
                generateErrorResponse(502);
                return true;
            }
        }

        parsed_cgi_headers[key] = value;
    }

    if (parsed_cgi_headers.empty())
    {
        generateErrorResponse(502);
        return true;
    }

    std::string http_headers = translateToHttp(parsed_cgi_headers);
    m_body_buffer.insert(m_body_buffer.end(), http_headers.begin(), http_headers.end());

    if (!leftover_body.empty())
    {
        if (m_is_chunked_response)
        {
            std::ostringstream hex_size;
            hex_size << std::hex << leftover_body.size() << "\r\n";
            std::string chunk_hdr = hex_size.str();

            m_body_buffer.insert(m_body_buffer.end(), chunk_hdr.begin(), chunk_hdr.end());
            m_body_buffer.insert(m_body_buffer.end(), leftover_body.begin(), leftover_body.end());
            m_body_buffer.push_back('\r');
            m_body_buffer.push_back('\n');
        }
        else
            m_body_buffer.insert(m_body_buffer.end(), leftover_body.begin(), leftover_body.end());
    }

    m_cgi_header_buffer.clear();
    return true;
}

/**
 * appendTerminalChunk - Pushes the final EOF chunk into the output queue.
 */
void CGIResponse::appendTerminalChunk()
{
    if (m_error_code != 0)
        return;

    // If the CGI script is still in the header reading phase or hasn't started streaming, we can't append a terminal
    // chunk so we send a clean error
    if (m_cgi_state == CGI_IDLE || m_cgi_state == CGI_READING_HEADERS)
    {
        generateErrorResponse(502);
        return;
    }

    if (m_cgi_state == CGI_STREAMING_BODY)
    {
        m_cgi_state = CGI_COMPLETE;

        if (m_is_chunked_response)
        {
            std::string term = "0\r\n\r\n";
            m_body_buffer.insert(m_body_buffer.end(), term.begin(), term.end());
        }
    }
}

/**
 * generateErrorResponse - Clears partial output and injects an HTTP error response into the buffer.
 * @error_code: The HTTP error code to include in the response.
 */
void CGIResponse::generateErrorResponse(int error_code)
{
    if (error_code == 0)
        return;

    m_body_buffer.clear();
    m_cgi_state  = CGI_COMPLETE;
    m_error_code = error_code;
}

void CGIResponse::consumeBodyChunk(size_t length)
{
    if (length >= m_body_buffer.size())
    {
        m_body_buffer.clear();
    }
    else
    {
        m_body_buffer.erase(m_body_buffer.begin(), m_body_buffer.begin() + static_cast<long>(length));
    }
    m_already_send_count += length;
}
