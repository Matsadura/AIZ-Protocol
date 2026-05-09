#include "Request.hpp"
#include <cstddef>
#include <cstdlib>
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
void Request::appendDataAndParse(const char *data, size_t length)
{
    m_raw_buffer.append(data, length);
    parseRequestLine();
    parseHeaders();
    // parseBody();
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
 * Parse the request line of the HTTP request, extracting the method, URI, and version.
 */
void Request::parseRequestLine()
{
    if (m_state != REQUEST_LINE)
        return;

    size_t pos = m_raw_buffer.find(CRLF);
    if (pos == std::string::npos)
        return;

    std::string request_line = m_raw_buffer.substr(0, pos);

    if (!validateRequestLineFormat(request_line))
        return;

    m_raw_buffer.erase(0, pos + 2);

    std::vector<std::string> tokens = split(request_line, ' ');
    if (!validateRequestLinePartsCount(tokens))
        return;

    m_method  = tokens[0];
    m_uri     = tokens[1];
    m_version = tokens[2];

    if (!validateMethod())
        return;

    if (!validateURI())
        return;

    if (!validateVersion())
        return;

    m_state = HEADERS;
}

/**
 * Parse the headers of the HTTP request, extracting key-value pairs and validating them.
 */
void Request::parseHeaders(void)
{
    if (m_state != HEADERS)
        return;

    size_t start_pos = 0;

    while (true)
    {
        size_t pos = m_raw_buffer.find(CRLF, start_pos);

        if (pos == std::string::npos)
            return;

        std::string line = m_raw_buffer.substr(start_pos, pos - start_pos);
        start_pos        = pos + 2;

        if (line.empty())
        {
            m_raw_buffer.erase(0, start_pos);
            break;
        }

        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos)
        {
            setError(BAD_REQUEST);
            return;
        }

        std::string key   = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);

        if (!validateHeaderKeyFormat(key))
            return;

        key   = toLower(key);
        value = trim(value);

        if (!validateHeaderKeyNotEmpty(key))
            return;

        if (!validateHeaderNameCharacters(key))
            return;

        if (!validateHeaderValue(value))
            return;

        if (!validateHeaderHost(key, value))
            return;

        if (!validateHeaderDuplicates(key, value))
            return;

        if (!validateHeaderContentLength(key, value))
            return;
    }

    if (!validateHTTP11Host())
        return;

    if (m_headers.find("content-length") != m_headers.end())
    {
        if (!m_headers["content-length"].empty())
        {
            m_content_length = std::strtoul(m_headers["content-length"].c_str(), NULL, 10);
        }
    }

    m_is_chunked = isBodyChunked();

    if (m_is_chunked || m_content_length > 0)
        m_state = BODY;
    else
        m_state = COMPLETE;
}

/**
 * Parse the body of the HTTP request, handling both chunked and non-chunked bodies.
 */
void Request::parseBody(void)
{
    if (m_state != BODY)
        return;

    if (m_is_chunked)
    {
        // parseChunkedBody();
        return;
    }

    if (!validateBodySize(m_raw_buffer.size()))
        return;

    size_t bytes_needed = m_content_length - m_body.size();
    if (m_raw_buffer.size() < bytes_needed)
    {
        m_body.insert(m_body.end(), m_raw_buffer.begin(), m_raw_buffer.end());
        m_raw_buffer.clear();
        return;
    }

    m_body.insert(m_body.end(), m_raw_buffer.begin(),
                  m_raw_buffer.begin() + static_cast<std::string::difference_type>(bytes_needed));
    m_raw_buffer.erase(0, bytes_needed);
    m_state = COMPLETE;
}
