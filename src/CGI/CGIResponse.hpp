#pragma once

class CGIResponse
{
  public:
    enum CgiState
    {
        CGI_IDLE,
        CGI_READING_HEADERS,
        CGI_STREAMING_BODY,
        CGI_COMPLETE
    };

    void        appendCgiData(const char *data, size_t length);
    bool        parseCgiHeaders();
    std::string getOutputChunk();

  private:
    CgiState          m_cgi_state;
    std::string       m_cgi_header_buffer;
    std::string       m_http_response_headers;
    std::vector<char> m_body_buffer;
    bool              m_is_local_redirect;
};