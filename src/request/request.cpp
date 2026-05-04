#include "request.hpp"
#include <cstddef>
#include <vector>

/**
 * Constructor for the Request class.
 */
Request::Request(void) :
    m_state(REQUEST_LINE),
    m_error_code(0),
    m_max_body_size(1024 * 1024),
    m_is_chunked(false),
    m_content_length(0),
    m_chunk_state(CHUNK_SIZE),
    m_current_chunk_size(0),
    m_chunk_bytes_read(0)
{
}

/**
 * Copy constructor for the Request class.
 * @other: The Request object to copy from.
 */
Request::Request(const Request &other) :
    m_state(other.m_state),
    m_raw_buffer(other.m_raw_buffer),
    m_error_code(other.m_error_code),
    m_max_body_size(other.m_max_body_size),
    m_method(other.m_method),
    m_uri(other.m_uri),
    m_path(other.m_path),
    m_query(other.m_query),
    m_version(other.m_version),
    m_headers(other.m_headers),
    m_body(other.m_body),
    m_is_chunked(other.m_is_chunked),
    m_content_length(other.m_content_length),
    m_chunk_state(other.m_chunk_state),
    m_current_chunk_size(other.m_current_chunk_size),
    m_chunk_bytes_read(other.m_chunk_bytes_read)
{
}

/**
 * Assignment operator for the Request class.
 * @other: The Request object to copy from.
 * Return: A reference to the current Request object.
 */
Request &Request::operator=(const Request &other)
{
    if (this != &other)
    {
        m_state              = other.m_state;
        m_raw_buffer         = other.m_raw_buffer;
        m_error_code         = other.m_error_code;
        m_max_body_size      = other.m_max_body_size;
        m_method             = other.m_method;
        m_uri                = other.m_uri;
        m_path               = other.m_path;
        m_query              = other.m_query;
        m_version            = other.m_version;
        m_headers            = other.m_headers;
        m_body               = other.m_body;
        m_is_chunked         = other.m_is_chunked;
        m_content_length     = other.m_content_length;
        m_chunk_state        = other.m_chunk_state;
        m_current_chunk_size = other.m_current_chunk_size;
        m_chunk_bytes_read   = other.m_chunk_bytes_read;
    }
    return *this;
}

/**
 * Destructor for the Request class.
 */
Request::~Request(void)
{
}

/**
 * Append data to the raw buffer of the request.
 * @data: Pointer to the data to append.
 * @length: Length of the data to append.
 */
void Request::appendData(const char *data, size_t length)
{
    m_raw_buffer.append(data, length);
}

/**
 * Set the maximum allowed body size for the request.
 * @max_size: The maximum body size in bytes.
 */
void Request::setMaxBodySize(size_t max_size)
{
    m_max_body_size = max_size;
}

/**
 * Get the current state of the request parser.
 * Return: The current ParserState of the request.
 */
Request::ParserState Request::getState() const
{
    return m_state;
}

/**
 * Get the error code if the request parsing failed.
 * Return: The error code (0 if no error).
 */
int Request::getErrorCode() const
{
    return m_error_code;
}

/**
 * Check if the request has been fully parsed and is complete.
 * Return: True if the request is complete, false otherwise.
 */
bool Request::isComplete() const
{
    return m_state == COMPLETE;
}

/**
 * Get the HTTP method of the request (e.g., GET, POST).
 * Return: The HTTP method as a string.
 */
std::string Request::getMethod() const
{
    return m_method;
}

/**
 * Get the URI of the request (e.g., /index.html).
 * Return: The URI as a string.
 */
std::string Request::getURI() const
{
    return m_uri;
}

/**
 * Get the HTTP version of the request (e.g., HTTP/1.1).
 * Return: The HTTP version as a string.
 */
std::string Request::getVersion() const
{
    return m_version;
}

/**
 * Get the value of a specific header from the request.
 * @key: The name of the header to retrieve.
 * Return: The value of the header as a string, or an empty string if not found.
 */
std::string Request::getHeader(const std::string &key) const
{
    std::map<std::string, std::string>::const_iterator it = m_headers.find(key);
    if (it != m_headers.end())
        return it->second;
    return "";
}

/**
 * Get all headers from the request as a map.
 * Return: A map of header names to their corresponding values.
 */
std::map<std::string, std::string> Request::getHeaders() const
{
    return m_headers;
}

/**
 * Get the body of the request as a vector of characters.
 * Return: A reference to the vector containing the request body.
 */
const std::vector<char> &Request::getBody() const
{
    return m_body;
}

/**
 * Set an error state for the request parser with a specific error code.
 * @error_code: The error code to set (e.g., BAD_REQUEST).
 */
void Request::setError(int error_code)
{
    m_state      = ERROR;
    m_error_code = error_code;
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
 * Parse the request line of the HTTP request, extracting the method, URI, and version.
 */
void Request::parseRequestLine()
{
    if (m_state != REQUEST_LINE)
        return;

    size_t pos = m_raw_buffer.find(CRLF);
    if (pos == std::string::npos)
        return;
    for (size_t i = 0; i < pos; ++i)
    {
        if (m_raw_buffer[i] == '\r' || m_raw_buffer[i] == '\n')
        {
            setError(BAD_REQUEST);
            return;
        }
    }

    std::string request_line = m_raw_buffer.substr(0, pos);
    m_raw_buffer.erase(0, pos + 2);

    if (request_line.empty())
    {
        setError(BAD_REQUEST);
        return;
    }

    std::vector<std::string> tokens = split(request_line, ' ');
    if (tokens.size() != 3)
    {
        setError(BAD_REQUEST);
        return;
    }
    m_method  = tokens[0];
    m_uri     = tokens[1];
    m_version = tokens[2];

    if (isAllUpper(m_method) == false)
    {
        setError(BAD_REQUEST);
        return;
    }

    if (m_method != "GET" && m_method != "POST" && m_method != "DELETE")
    {
        const std::string unsupported_methods[] = {"PUT", "HEAD", "PATCH", "OPTIONS", "TRACE", "CONNECT"};
        for (size_t i = 0; i < sizeof(unsupported_methods) / sizeof(unsupported_methods[0]); ++i)
        {
            if (m_method == unsupported_methods[i])
            {
                setError(NOT_IMPLEMENTED);
                return;
            }
        }
        setError(BAD_REQUEST);
        return;
    }

    if (m_uri.empty() || m_uri[0] != '/')
    {
        setError(BAD_REQUEST);
        return;
    }
    for (size_t i = 0; i < m_uri.size(); ++i)
    {
        if (m_uri[i] <= 31 || m_uri[i] == 127)
        {
            setError(BAD_REQUEST);
            return;
        }
    }
    std::string decoded_uri;
    if (!decodeURI(m_uri, decoded_uri))
    {
        setError(BAD_REQUEST);
        return;
    }
    m_uri = decoded_uri;
    extractPathAndQuery();

    if (m_version != "HTTP/1.0" && m_version != "HTTP/1.1")
    {
        setError(BAD_REQUEST);
        return;
    }

    m_state = HEADERS;
}

void Request::parseHeaders(void)
{
    if (m_state != HEADERS)
        return;

    while (true)
    {
        size_t pos = m_raw_buffer.find(CRLF);
        if (pos == std::string::npos)
            return;

        std::string line = m_raw_buffer.substr(0, pos);
        m_raw_buffer.erase(0, pos + 2);

        if (line.empty())
        {
            m_state = BODY;
            return;
        }

        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos)
        {
            setError(BAD_REQUEST);
            return;
        }

        std::string key   = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);

        if (key.empty())
        {
            setError(BAD_REQUEST);
            return;
        }

        std::string host = toLower() if (toLower(key) == "Host") // Host header is mandatory in HTTP/1.1 (unfinished)

            key = trim(key);
        value   = trim(value);
        key     = toLower(key);

        if (isDuplicateHeader(m_headers, key))
        {
            setError(BAD_REQUEST);
            return;
        }

        if (key == "content-length")
        {
            if (isNumeric(value) == false)
            {
                setError(BAD_REQUEST);
                return;
            }
        }

        m_headers[key] = value;
    }
}
