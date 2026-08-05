#include "Request.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <unistd.h>
#include <vector>

/**
 * Constructor for the Request class.
 */
Request::Request(void) :
    m_state(REQUEST_LINE),
    m_error_code(0),
    m_max_body_size(1024 * 1024 * 1042),
    m_body_fd(-1),
    m_body_bytes_read(0),
    m_is_generated_temp_file(false),
    m_is_chunked(false),
    m_content_length(0),
    m_chunk_state(CHUNK_SIZE),
    m_chunk_bytes_remaining(0)
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
    m_body_fd(-1),
    m_body_filename(other.m_body_filename),
    m_body_bytes_read(other.m_body_bytes_read),
    m_is_generated_temp_file(other.m_is_generated_temp_file),
    m_is_chunked(other.m_is_chunked),
    m_content_length(other.m_content_length),
    m_chunk_state(other.m_chunk_state),
    m_chunk_bytes_remaining(other.m_chunk_bytes_remaining)
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
        m_state                 = other.m_state;
        m_raw_buffer            = other.m_raw_buffer;
        m_error_code            = other.m_error_code;
        m_max_body_size         = other.m_max_body_size;
        m_method                = other.m_method;
        m_uri                   = other.m_uri;
        m_path                  = other.m_path;
        m_query                 = other.m_query;
        m_version               = other.m_version;
        m_headers               = other.m_headers;
        m_body_filename         = other.m_body_filename;
        m_is_chunked            = other.m_is_chunked;
        m_content_length        = other.m_content_length;
        m_chunk_state           = other.m_chunk_state;
        m_chunk_bytes_remaining = other.m_chunk_bytes_remaining;
    }
    return *this;
}

/**
 * Destructor for the Request class.
 */
Request::~Request(void)
{
    if (m_body_fd != -1)
    {
        close(m_body_fd);
    }
    if (m_is_generated_temp_file && !m_body_filename.empty())
    {
        std::remove(m_body_filename.c_str());
    }
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
    parseBody();
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
 * Get the path component of the request URI.
 * Return: The path as a string.
 */
std::string Request::getPath() const
{
    return m_path;
}

/**
 * Get the query string of the request URI.
 * Return: The query string as a string.
 */
std::string Request::getQuery() const
{
    return m_query;
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
 * Get the raw buffer containing the unprocessed request data.
 * Return: A reference to the string containing the raw request data.
 */
const std::string &Request::getRawBuffer() const
{
    return m_raw_buffer;
}

/**
 * Get the filename of the request body file.
 * Return: The filename as a string.
 */
const std::string &Request::getBodyFilename() const
{
    return m_body_filename;
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
 * Reset the request parser to its initial state, clearing all data and errors.
 */
void Request::reset(int reset_type)
{
    m_state = REQUEST_LINE;
    if (reset_type == FULL_RESET)
        m_raw_buffer.clear();
    m_error_code = 0;
    m_method.clear();
    m_uri.clear();
    m_path.clear();
    m_query.clear();
    m_version.clear();
    m_headers.clear();

    if (m_body_fd != -1)
    {
        close(m_body_fd);
        m_body_fd = -1;
    }
    if (m_is_generated_temp_file && !m_body_filename.empty())
    {
        std::remove(m_body_filename.c_str());
    }
    m_body_filename.clear();
    m_is_generated_temp_file = false;
    m_is_chunked             = false;
    m_content_length         = 0;
    m_chunk_state            = CHUNK_SIZE;
    m_chunk_bytes_remaining  = 0;
    m_body_bytes_read        = 0;
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
    {
        if (m_raw_buffer.size() > 8192)
            setError(URI_TOO_LONG);
        return;
    }

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
        {
            if (m_raw_buffer.size() - start_pos > 8192)
                setError(HEADER_TOO_LARGE);
            return;
        }

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

    parseContentLengthHeader();

    bool has_content_length    = m_headers.find("content-length") != m_headers.end();
    bool has_transfer_encoding = m_headers.find("transfer-encoding") != m_headers.end();
    if (has_transfer_encoding && !validateTransferEncoding(m_headers["transfer-encoding"]))
        return;

    if (m_is_chunked || has_content_length || has_transfer_encoding)
        m_state = HEADERS_COMPLETE;
    else
        m_state = COMPLETE;
}

/**
 * Parse the body of the HTTP request when Transfer-Encoding is set to chunked.
 */
void Request::parseChunkedBody(void)
{
    while (!m_raw_buffer.empty() && m_state == BODY)
    {
        switch (m_chunk_state)
        {
            case CHUNK_SIZE:
                if (!parseChunkSize())
                    return;
                break;
            case CHUNK_DATA:
                if (!parseChunkData())
                    return;
                break;
            case CHUNK_DATA_CRLF:
                if (!parseChunkDataCRLF())
                    return;
                break;
            case CHUNK_TRAILER:
                if (!parseChunkTrailer())
                    return;
                break;
        }
    }
}

/**
 * Parse the body of the HTTP request when Transfer-Encoding is not set to chunked.
 */
bool Request::parseUnchunkedBody()
{
    size_t body_bytes_needed = m_content_length - m_body_bytes_read;
    size_t bytes_to_append   = std::min(body_bytes_needed, m_raw_buffer.size());

    if (!validateBodySize(bytes_to_append))
        return false;

    if (m_body_fd != -1)
    {
        if (write(m_body_fd, m_raw_buffer.c_str(), bytes_to_append) != static_cast<long>(bytes_to_append))
        {
            setError(500);
            return false;
        }
    }

    m_raw_buffer.erase(0, bytes_to_append);
    m_body_bytes_read += bytes_to_append;

    if (m_body_bytes_read >= m_content_length)
    {
        if (m_body_fd != -1)
        {
            close(m_body_fd);
            m_body_fd = -1;
        }
        m_state = COMPLETE;
        return true;
    }

    return false;
}

/**
 * Parse the body of the HTTP request, handling both chunked and non-chunked bodies.
 */
void Request::parseBody(void)
{
    if (m_state != BODY)
        return;

    if (!validateBodyHeaders())
        return;

    if (m_is_chunked)
    {
        parseChunkedBody();
        return;
    }

    if (!parseUnchunkedBody())
        return;
    m_state = COMPLETE;
}
