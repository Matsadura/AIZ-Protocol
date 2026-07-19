#ifndef CGI_OUTPUT_PARSER_HPP
#define CGI_OUTPUT_PARSER_HPP

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

class CgiOutputParser
{
  public:
    explicit CgiOutputParser();
    ~CgiOutputParser();

    // Appends raw data from the CGI script to the internal parsing buffer.
    void append(const char *data, std::size_t size);

    // Signals that the CGI script has closed its stdout (EOF).
    void setEof();

    // Indicates if the intermediate body buffer has reached or exceeded its limit.
    bool isBufferFull() const;

    // Indicates if the entire CGI response has been parsed and fully consumed.
    bool isComplete() const;

    // Clears all internal state and buffers, making the object ready for a new request.
    void reset();

    // Header inspection
    bool               areHeadersParsed() const;
    const std::string &getHeaders() const;

    // Zero-copy body consumption mechanism:
    // 1. Call getBodyData() to get a pointer and size for your write() call.
    // 2. Call consume() with the actual number of bytes written to remove them.
    void getBodyData(const char *&out_data, std::size_t &out_size) const;
    void consume(std::size_t written);

  private:
    void process();

    enum State
    {
        STATE_HEADERS,
        STATE_BODY,
        STATE_FINISHED
    };

    State       m_state;
    std::size_t m_max_buffer_size;
    bool        m_eof_signaled;

    std::vector<char> m_raw_buffer;
    std::vector<char> m_body_buffer;
    std::string       m_headers;
};

#endif
