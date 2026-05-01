#include "../includes/request.hpp"

/**
 * Constructor for the Request class.
 */
Request::Request( void ) :
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
 */
Request::Request(const Request& other) :
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
 */
Request& Request::operator=(const Request& other)
{
    if (this != &other)
    {
        m_state = other.m_state;
        m_raw_buffer = other.m_raw_buffer;
        m_error_code = other.m_error_code;
        m_max_body_size = other.m_max_body_size;
        m_method = other.m_method;
        m_uri = other.m_uri;
        m_path = other.m_path;
        m_query = other.m_query;
        m_version = other.m_version;
        m_headers = other.m_headers;
        m_body = other.m_body;
        m_is_chunked = other.m_is_chunked;
        m_content_length = other.m_content_length;
        m_chunk_state = other.m_chunk_state;
        m_current_chunk_size = other.m_current_chunk_size;
        m_chunk_bytes_read = other.m_chunk_bytes_read;
    }
    return *this;
}

/**
 * Destructor for the Request class.
 */
Request::~Request( void )
{
}