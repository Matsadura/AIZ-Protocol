#include "CGIResponse.hpp"
#include <sstream>

CGIResponse::CGIResponse() : m_cgi_state(CGI_IDLE), m_is_local_redirect(false), m_error_code(0)
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

/**
 * appendCgiData - Appends CGI data to the internal buffer.
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
            m_cgi_state = CGI_STREAMING_BODY;
    }
    else if (m_cgi_state == CGI_STREAMING_BODY)
        m_body_buffer.insert(m_body_buffer.end(), data, data + length);
}

/**
 * translateToHttp - Translates CGI headers to HTTP response headers.
 * @cgi_headers: A map containing CGI headers to be translated.
 */
void CGIResponse::translateToHttp(std::map<std::string, std::string> &cgi_headers)
{
    std::ostringstream http_headers;
    std::string        status = "200 OK";

    if (cgi_headers.count("status"))
        status = cgi_headers["status"];
    if (cgi_headers.count("location"))
    {
        std::string loc = cgi_headers["location"];
        if (!loc.empty() && loc[0] == '/')
            m_is_local_redirect = true;
        else
            status = "302 Found";
    }

    http_headers << "HTTP/1.1 " << status << "\r\n";

    if (cgi_headers.count("content-type"))
        http_headers << "Content-Type: " << cgi_headers["content-type"] << "\r\n";

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

/**
 * parseCgiHeaders - Parses the CGI headers from the buffer.
 * Return: true if headers are complete and valid, false if more data is needed or if there's an error.
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

    m_body_buffer.insert(m_body_buffer.end(), leftover_body.begin(), leftover_body.end());

    std::vector<std::string>           header_lines = split(headers_part, '\n');
    std::map<std::string, std::string> parsed_cgi_headers;

    for (std::vector<std::string>::const_iterator it = header_lines.begin(); it != header_lines.end(); ++it)
    {
        std::string line = *it;

        // Remove trailing \r if it exists (since we split by \n)
        if (!line.empty() && line[line.length() - 1] == '\r')
            line.erase(line.length() - 1);

        if (line.empty())
            continue;

        // RULE: CGI/1.1 does not support continuation lines
        if (line[0] == ' ' || line[0] == '\t')
        {
            m_error_code = 502;
            return true;
        }

        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos)
        {
            m_error_code = 502;
            return true;
        }

        std::string key   = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);

        // RULE: No whitespace between field name and colon
        if (!key.empty() && (key[key.length() - 1] == ' ' || key[key.length() - 1] == '\t'))
        {
            m_error_code = 502;
            return true;
        }

        trim(key);
        trim(value);
        // RULE: case insensitive field names, but we will store them in lowercase for consistency
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

        if (!isValidHeaderName(key) || !isValidHeaderValue(value))
        {
            m_error_code = 502;
            return true;
        }

        // RULE: NULL field value is equivalent to a field not being sent
        if (value.empty())
            continue;

        // RULE: CGI fields MUST NOT appear more than once
        if (key == "content-type" || key == "location" || key == "status")
        {
            if (parsed_cgi_headers.count(key) > 0)
            {
                m_error_code = 502;
                return true;
            }
        }

        parsed_cgi_headers[key] = value;
    }

    // RULE: At least one CGI field MUST be supplied
    if (parsed_cgi_headers.empty())
    {
        m_error_code = 502;
        return true;
    }

    translateToHttp(parsed_cgi_headers);
    return true;
}

/**
 * getOutputChunk - Retrieves the next chunk of output to be sent to the client.
 * Return: A string containing the next chunk of output, or an empty string if there's no more data.
 */
std::string CGIResponse::getOutputChunk()
{
    if (m_error_code != 0)
        return "";

    std::string output;

    if (!m_http_response_headers.empty())
    {
        output += m_http_response_headers;
        m_http_response_headers.clear();
    }

    if (!m_body_buffer.empty())
    {
        std::ostringstream hex_size;
        hex_size << std::hex << m_body_buffer.size() << "\r\n";

        output += hex_size.str();
        output.append(m_body_buffer.begin(), m_body_buffer.end());
        output += "\r\n";

        m_body_buffer.clear();
    }

    return output;
}

/**
 * getTerminalChunk - Retrieves the terminal chunk indicating the end of the response.
 * Return: A string containing the terminal chunk, or an empty string if not applicable.
 */
std::string CGIResponse::getTerminalChunk()
{
    if (m_cgi_state == CGI_STREAMING_BODY && m_error_code == 0)
    {
        m_cgi_state = CGI_COMPLETE;
        return "0\r\n\r\n";
    }
    return "";
}

/**
 * generateErrorResponse - Generates a simple HTTP error response for the given error code.
 * @error_code: The HTTP error code to include in the response.
 * Return: A string containing the HTTP error response.
 */
std::string CGIResponse::generateErrorResponse(int error_code)
{
    if (error_code == 0)
        return "";
    std::ostringstream response;
    response << "HTTP/1.1 " << error_code << " Error\r\n";
    response << "Content-Type: text/plain\r\n";
    response << "Content-Length: 0\r\n";
    response << "\r\n";
    return response.str();
}