#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../utils/utils.hpp"

#define CRLF "\r\n"
#define BAD_REQUEST 400
#define METHOD_NOT_ALLOWED 405
#define PAYLOAD_TOO_LARGE 413
#define HEADER_TOO_LARGE 431
#define NOT_IMPLEMENTED 501

class Request
{
  public:
    enum ParserState
    {
        REQUEST_LINE,
        HEADERS,
        BODY,
        COMPLETE,
        ERROR
    };

    enum ChunkState
    {
        CHUNK_SIZE,
        CHUNK_DATA,
        CHUNK_TRAILER
    };

    Request(void);
    Request(const Request &other);
    Request &operator=(const Request &other);
    ~Request(void);

    void appendDataAndParse(const char *data, size_t length);
    void setMaxBodySize(size_t max_size);

    void setError(int error_code);
    ParserState getState(void) const;
    int getErrorCode(void) const;
    bool isComplete(void) const;

    std::string getMethod(void) const;
    std::string getURI(void) const;
    std::string getVersion(void) const;
    std::string getHeader(const std::string &key) const;
    std::map<std::string, std::string> getHeaders(void) const;
    const std::vector<char> &getBody(void) const;

  private:
    ParserState m_state;
    std::string m_raw_buffer;
    int m_error_code;
    size_t m_max_body_size;

    std::string m_method;
    std::string m_uri;
    std::string m_path;
    std::string m_query;
    std::string m_version;
    std::map<std::string, std::string> m_headers;
    std::vector<char> m_body;

    bool m_is_chunked;
    size_t m_content_length;

    ChunkState m_chunk_state;
    size_t m_current_chunk_size;
    size_t m_chunk_bytes_read;

    /* Core parsing functions */

    void parseRequestLine(void);
    void parseHeaders(void);
    void parseBody(void);
    void parseChunkedBody(void);

    /* Helper functions for request line parsing and validation */

    bool decodeURI(const std::string &uri, std::string &decoded_uri);
    void extractPathAndQuery(void);
    bool validateRequestLineFormat(const std::string &line);
    bool validateRequestLinePartsCount(const std::vector<std::string> &request_line_tokens);
    bool validateMethod(void);
    bool validateURI(void);
    bool validateVersion(void);

    /* Helper functions for header validation and parsing */

    bool validateHeaderKeyFormat(const std::string &key);
    bool validateHeaderKeyNotEmpty(const std::string &key);
    bool validateHeaderNameCharacters(const std::string &key);
    bool validateHeaderValue(const std::string &value);
    bool validateHeaderHost(const std::string &key, const std::string &value);
    bool validateHeaderDuplicates(const std::string &key, std::string &value);
    bool validateContentLengthOverflow(const std::string &value, size_t &result);
    bool validateHeaderContentLength(const std::string &key, const std::string &value);
    bool validateHTTP11Host(void);
    bool isBodyChunked(void) const;
    void parseContentLengthHeader(void);

    /* Helper functions for body parsing */
    bool validateBodyHeaders(void);
    bool validateBodySize(size_t new_data_size);
};

#endif /* REQUEST_HPP */