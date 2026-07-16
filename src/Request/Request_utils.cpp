#include "Request.hpp"
#include <cerrno>

/**
 * Check if the request is ready for body parsing.
 * @state: The current state of the request.
 * Return: True if the request is ready for body parsing, false otherwise.
 */
bool Request::isReadyForBodyParsing(bool yes_no)
{
    if (yes_no == true)
    {
        m_state = BODY;
        return true;
    }
    return false;
}

/**
 * Check if the request is ready for routing.
 * Return: True if the request is ready for routing, false otherwise.
 */
bool Request::isReadyForRouting(void) const
{
    if (m_state == HEADERS_COMPLETE || m_state == COMPLETE)
        return true;
    return false;
}

/**
 * Consume a chunk of the request body.
 */
void Request::consumeBodyChunk(void)
{
    m_body.clear();

    if (m_state == BODY_CHUNK_READY)
        m_state = BODY;
}

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
 * Validate the Transfer-Encoding header value to ensure it contains "chunked" if present.
 * @value: The value of the Transfer-Encoding header to validate.
 * Return: True if the Transfer-Encoding value is valid (contains "chunked"), false otherwise.
 */
bool Request::validateTransferEncoding(const std::string &value)
{
    std::vector<std::string> encodings = split(toLower(value), ',');

    if (encodings.empty())
    {
        setError(BAD_REQUEST);
        return false;
    }

    bool found_chunked = false;

    for (size_t i = 0; i < encodings.size(); ++i)
    {
        std::string encoding = trim(encodings[i]);

        if (encoding.empty())
        {
            setError(BAD_REQUEST);
            return false;
        }

        if (encoding == "chunked")
        {
            if (i != encodings.size() - 1)
            {
                setError(BAD_REQUEST);
                return false;
            }

            if (found_chunked)
            {
                setError(BAD_REQUEST);
                return false;
            }

            found_chunked = true;
        }
    }

    if (!found_chunked)
    {
        setError(NOT_IMPLEMENTED);
        return false;
    }

    m_is_chunked = true;
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
    return true;
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
 * Validate that the body size does not exceed:
 * - the announced Content-Length
 * - the server maximum body size
 *
 * @new_data_size: Number of bytes about to be appended.
 * Return: True if valid, false otherwise.
 */
bool Request::validateBodySize(size_t new_data_size)
{
    size_t total_projected_size = m_body_bytes_read + new_data_size;

    if (!m_is_chunked && total_projected_size > m_content_length)
    {
        setError(BAD_REQUEST);
        return false;
    }

    if (total_projected_size > m_max_body_size)
    {
        setError(PAYLOAD_TOO_LARGE);
        return false;
    }

    return true;
}

bool Request::validateChunkSizeFormat(const std::string &line)
{
    if (line.empty() || line.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
    {
        setError(BAD_REQUEST);
        return false;
    }

    char *endptr = NULL;
    errno        = 0;
    std::strtoul(line.c_str(), &endptr, 16);
    if (*endptr != '\0' || errno == ERANGE)
    {
        setError(BAD_REQUEST);
        return false;
    }
    return true;
}

bool Request::parseChunkSize(void)
{
    size_t crlf_pos = m_raw_buffer.find("\r\n");
    if (crlf_pos == std::string::npos)
        return false;

    std::string size_line = m_raw_buffer.substr(0, crlf_pos);
    size_t      semi_pos  = size_line.find(';');
    std::string size_str  = (semi_pos == std::string::npos) ? size_line : size_line.substr(0, semi_pos);

    if (!validateChunkSizeFormat(size_str))
        return false;

    m_chunk_bytes_remaining = std::strtoul(size_str.c_str(), NULL, 16);
    m_raw_buffer.erase(0, crlf_pos + 2);
    m_chunk_state = (m_chunk_bytes_remaining == 0) ? CHUNK_TRAILER : CHUNK_DATA;
    return true;
}

bool Request::parseChunkData(void)
{
    if (m_raw_buffer.empty())
        return false;

    size_t bytes_to_append = std::min(m_chunk_bytes_remaining, m_raw_buffer.size());

    if (!validateBodySize(bytes_to_append))
        return false;

    size_t available_space = CHUNK_SIZE_LIMIT - m_body.size();
    if (bytes_to_append > available_space)
        bytes_to_append = available_space;

    m_body.insert(m_body.end(), m_raw_buffer.begin(),
                  m_raw_buffer.begin() + static_cast<std::string::difference_type>(bytes_to_append));
    m_raw_buffer.erase(0, bytes_to_append);
    m_chunk_bytes_remaining -= bytes_to_append;

    m_body_bytes_read += bytes_to_append;

    if (m_body.size() >= CHUNK_SIZE_LIMIT)
    {
        m_state = BODY_CHUNK_READY;
        return false;
    }

    if (m_chunk_bytes_remaining == 0)
        m_chunk_state = CHUNK_DATA_CRLF;
    return true;
}

bool Request::parseChunkDataCRLF(void)
{
    if (m_raw_buffer.size() < 2)
        return false;

    if (m_raw_buffer[0] != '\r' || m_raw_buffer[1] != '\n')
    {
        setError(BAD_REQUEST);
        return false;
    }

    m_raw_buffer.erase(0, 2);
    m_chunk_state = CHUNK_SIZE;
    return true;
}

bool Request::parseChunkTrailer()
{
    while (true)
    {
        size_t pos = m_raw_buffer.find(CRLF);

        if (pos == std::string::npos)
            return false;

        std::string line = m_raw_buffer.substr(0, pos);

        m_raw_buffer.erase(0, pos + 2);

        if (line.empty())
        {
            m_state = COMPLETE;
            return true;
        }

        size_t colon_pos = line.find(':');

        if (colon_pos == std::string::npos)
        {
            setError(BAD_REQUEST);
            return false;
        }

        std::string key   = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);

        if (!validateHeaderKeyFormat(key))
            return false;

        key   = toLower(key);
        value = trim(value);

        if (!validateHeaderKeyNotEmpty(key))
            return false;

        if (!validateHeaderNameCharacters(key))
            return false;

        if (!validateHeaderValue(value))
            return false;
    }
}
