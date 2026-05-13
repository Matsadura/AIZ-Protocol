#include "Request.hpp"

/**
 * Extract the path and query components from the URI.
 */
void Request::extractPathAndQuery(void)
{
    size_t pos = m_uri.find('?');
    if (pos != std::string::npos)
    {
        m_path  = m_uri.substr(0, pos);
        m_query = m_uri.substr(pos + 1);
    }
    else
    {
        m_path  = m_uri;
        m_query = "";
    }
}

/**
 * Decode a percent-encoded URI string.
 * @uri: The percent-encoded URI to decode.
 * @decoded_uri: Reference to a string where the decoded URI will be stored.
 * Return: True if decoding was successful, false if there was an error (e.g., invalid encoding).
 */
bool Request::decodeURI(const std::string &uri, std::string &decoded_uri)
{
    decoded_uri.clear();
    for (size_t i = 0; i < uri.length(); ++i)
    {
        if (uri[i] == '%')
        {
            if (i + 2 >= uri.length())
            {
                return false;
            }
            int high = hexToInt(uri[i + 1]);
            int low  = hexToInt(uri[i + 2]);
            if (high == -1 || low == -1)
            {
                return false;
            }
            decoded_uri += static_cast<char>(high * 16 + low);
            i += 2;
        }
        else
        {
            decoded_uri += uri[i];
        }
    }
    return true;
}

/**
 * Validate the number of parts in the request line (should be exactly 3: method, URI, version).
 * @request_line_tokens: The vector of tokens extracted from the request line.
 * Return: True if the number of parts is valid, false otherwise.
 */
bool Request::validateRequestLinePartsCount(const std::vector<std::string> &request_line_tokens)
{
    if (request_line_tokens.size() != 3)
    {
        setError(BAD_REQUEST);
        return false;
    }
    return true;
}

/**
 * Validate the request line format (no embedded CR/LF).
 * @request_line: The request line to validate.
 * Return: True if valid, false otherwise.
 */
bool Request::validateRequestLineFormat(const std::string &request_line)
{
    if (request_line.empty())
    {
        setError(BAD_REQUEST);
        return false;
    }

    for (size_t i = 0; i < request_line.size(); ++i)
    {
        if (request_line[i] == '\r' || request_line[i] == '\n')
        {
            setError(BAD_REQUEST);
            return false;
        }
    }
    return true;
}

/**
 * Validate the HTTP method if is one of the supported methods (GET, POST, DELETE) and is all uppercase.
 * Return: True if valid, false otherwise.
 */
bool Request::validateMethod()
{
    if (!isAllUpper(m_method))
    {
        setError(BAD_REQUEST);
        return false;
    }

    if (m_method != "GET" && m_method != "POST" && m_method != "DELETE")
    {
        const std::string unsupported_methods[] = {"PUT", "HEAD", "PATCH", "OPTIONS", "TRACE", "CONNECT"};
        for (size_t i = 0; i < sizeof(unsupported_methods) / sizeof(unsupported_methods[0]); ++i)
        {
            if (m_method == unsupported_methods[i])
            {
                setError(NOT_IMPLEMENTED);
                return false;
            }
        }
        setError(BAD_REQUEST);
        return false;
    }
    return true;
}

/**
 * Validate the URI if it starts with '/' and does not contain control characters, and decode percent-encoded
 * characters. Return: True if valid, false otherwise.
 */
bool Request::validateURI()
{
    if (m_uri.empty() || m_uri[0] != '/')
    {
        setError(BAD_REQUEST);
        return false;
    }

    for (size_t i = 0; i < m_uri.size(); ++i)
    {
        if (m_uri[i] <= 31 || m_uri[i] == 127)
        {
            setError(BAD_REQUEST);
            return false;
        }
    }

    std::string decoded_uri;
    if (!decodeURI(m_uri, decoded_uri))
    {
        setError(BAD_REQUEST);
        return false;
    }

    for (size_t i = 0; i < decoded_uri.size(); ++i)
    {
        if (decoded_uri[i] <= 31 || decoded_uri[i] == 127)
        {
            setError(BAD_REQUEST);
            return false;
        }
    }
    m_uri = decoded_uri;
    extractPathAndQuery();
    return true;
}

/**
 * Validate the HTTP version if it is either HTTP/1.0 or HTTP/1.1.
 * Return: True if valid, false otherwise.
 */
bool Request::validateVersion()
{
    if (m_version != "HTTP/1.0" && m_version != "HTTP/1.1")
    {
        setError(BAD_REQUEST);
        return false;
    }
    return true;
}

/**
 * Validate that the header key is not empty and has not invalid white space before trimming.
 * @key: The header key to validate.
 * Return: True if the header key is valid (not empty), false if it is empty.
 */
bool Request::validateHeaderKeyFormat(const std::string &key)
{
    if (!key.empty() && (key[key.size() - 1] == ' ' || key[key.size() - 1] == '\t'))
    {
        setError(BAD_REQUEST);
        return false;
    }
    return true;
}

/**
 * Validate that the header key is not empty after trimming.
 * @key: The header key to validate.
 * Return: True if the header key is valid (not empty), false if it is empty.
 */
bool Request::validateHeaderKeyNotEmpty(const std::string &key)
{
    if (key.empty())
    {
        setError(BAD_REQUEST);
        return false;
    }
    return true;
}

bool isTokenChar(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '!' || c == '#' ||
           c == '$' || c == '%' || c == '&' || c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' || c == '^' ||
           c == '_' || c == '`' || c == '|' || c == '~';
}

/**
 * Validate that the header key contains only valid characters (token characters as per RFC 7230).
 * @key: The header key to validate.
 * Return: True if the header key is valid, false if it contains invalid characters.
 */
bool Request::validateHeaderNameCharacters(const std::string &key)
{
    for (size_t i = 0; i < key.size(); ++i)
    {
        if (!isTokenChar(key[i]))
        {
            setError(BAD_REQUEST);
            return false;
        }
    }
    return true;
}

/**
 * Validate that the header value does not exceed a reasonable size limit (e.g., 8192 bytes).
 * @value: The header value to validate.
 * Return: True if the header value size is valid, false if it exceeds the limit.
 */
bool Request::validateHeaderValue(const std::string &value)
{
    for (size_t i = 0; i < value.size(); ++i)
    {
        if ((value[i] <= 31 && value[i] != '\t') || value[i] == 127)
        {
            setError(BAD_REQUEST);
            return false;
        }
    }
    if (value.size() > 8192)
    {
        setError(BAD_REQUEST);
        return false;
    }
    return true;
}

/**
 * Validate that the Host header exists and is not empty for HTTP/1.1 requests.
 * @key: The header key being validated.
 * @value: The header value being validated.
 * Return: True if the Host header is valid, false if it is invalid (e.g., empty for HTTP/1.1).
 */
bool Request::validateHeaderHost(const std::string &key, const std::string &value)
{
    if (m_version == "HTTP/1.1" && key == "host" && value.empty())
    {
        setError(BAD_REQUEST);
        return false;
    }
    return true;
}

/**
 * Validate that certain headers (like Host and Content-Length) are not duplicated, and merge others if they are.
 * @key: The header key being validated.
 * @value: The header value being validated.
 * Return: True if the header is valid and processed successfully, false if there was an error (e.g., duplicate
 * Host).
 */
bool Request::validateHeaderDuplicates(const std::string &key, std::string &value)
{
    if (m_headers.find(key) != m_headers.end())
    {
        if (key == "host")
        {
            setError(BAD_REQUEST);
            return false;
        }

        if (key == "content-length")
        {
            if (value != m_headers[key])
            {
                setError(BAD_REQUEST);
                return false;
            }

            return true;
        }

        m_headers[key] += ", " + value;
    }
    else
    {
        m_headers[key] = value;
    }

    return true;
}

/**
 * Validate that the Content-Length header value is numeric and does not exceed the maximum body size.
 * @key: The header key being validated.
 * @value: The header value being validated.
 * Return: True if the Content-Length value is valid, false if it is invalid (e.g., non-numeric or too large).
 */
bool Request::validateContentLengthOverflow(const std::string &value, size_t &result)
{
    if (value.empty())
        return false;

    result = 0;

    for (size_t i = 0; i < value.size(); ++i)
    {
        if (!std::isdigit(value[i]))
            return false;

        size_t digit = value[i] - '0';

        if (result > (std::numeric_limits<size_t>::max() - digit) / 10)
            return false;

        result = result * 10 + digit;
    }

    return true;
}

/**
 * Validate that the Content-Length header value is numeric if present.
 * @key: The header key being validated.
 * @value: The header value being validated.
 * Return: True if the Content-Length value is valid or if the header is not Content-Length, false if invalid.
 */
bool Request::validateHeaderContentLength(const std::string &key, const std::string &value)
{
    if (key != "content-length")
        return true;

    if (!isNumeric(value) || value.empty())
    {
        setError(BAD_REQUEST);
        return false;
    }

    size_t content_length = 0;
    if (!validateContentLengthOverflow(value, content_length))
    {
        setError(BAD_REQUEST);
        return false;
    }

    if (content_length > m_max_body_size)
    {
        setError(PAYLOAD_TOO_LARGE);
        return false;
    }

    return true;
}

/**
 * Validate that HTTP/1.1 requests contain a Host header.
 * Return: True if the Host header is present for HTTP/1.1 requests, false otherwise.
 */
bool Request::validateHTTP11Host(void)
{
    if (m_version == "HTTP/1.1" && m_headers.find("host") == m_headers.end())
    {
        setError(BAD_REQUEST);
        return false;
    }
    return true;
}

/**
 * Validate body-related headers according to RFC rules.
 *
 * Rules:
 * - Content-Length and Transfer-Encoding must not coexist.
 * - If body bytes are already present, the request must define
 *   the body length using either Content-Length or
 *   Transfer-Encoding: chunked.
 * - Content-Length: 0 is valid.
 *
 * Return: True if body headers are valid, false otherwise.
 */
bool Request::validateBodyHeaders(void)
{
    bool has_content_length = m_headers.find("content-length") != m_headers.end();

    bool has_transfer_encoding = m_headers.find("transfer-encoding") != m_headers.end();

    if (has_content_length && has_transfer_encoding)
    {
        setError(BAD_REQUEST);
        return false;
    }

    if (!m_raw_buffer.empty() && !has_content_length && !has_transfer_encoding)
    {
        setError(BAD_REQUEST);
        return false;
    }

    return true;
}

/**
 * Check if the request body is chunked based on the Transfer-Encoding header.
 * Return: True if the body is chunked, false otherwise.
 */
bool Request::isBodyChunked(void) const
{
    std::map<std::string, std::string>::const_iterator it = m_headers.find("transfer-encoding");
    if (it != m_headers.end())
    {
        std::string value                  = toLower(it->second);
        std::vector<std::string> encodings = split(value, ',');
        for (size_t i = 0; i < encodings.size(); ++i)
        {
            std::string encoding = trim(encodings[i]);
            if (encoding == "chunked")
                return true;
        }
    }
    return false;
}

void Request::parseContentLengthHeader(void)
{

    if (m_headers.find("content-length") != m_headers.end())
    {
        if (!m_headers["content-length"].empty())
        {
            m_content_length = std::strtoul(m_headers["content-length"].c_str(), NULL, 10);
        }
    }
}

/**
 * Validate that the body size does not exceed the Content-Length or the maximum body size limits.
 * @new_data_size: The size of the new data being added to the body.
 * Return: True if the body size is valid, false if it exceeds limits.
 */
bool Request::validateBodySize(size_t new_data_size)
{
    if (m_body.size() > m_content_length)
    {
        setError(BAD_REQUEST);
        return false;
    }

    if (m_body.size() + new_data_size > m_max_body_size)
    {
        setError(PAYLOAD_TOO_LARGE);
        return false;
    }
    return true;
}
