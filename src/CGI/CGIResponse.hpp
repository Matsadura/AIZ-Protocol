#pragma once
#include "../../src/core/Common.h"
#include "../../src/utils/utils.hpp"
#include <map>
#include <vector>

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
    std::string getTerminalChunk();

    void        setCgiState(CgiState state);
    CgiState    getCgiState() const;
    int         getErrorCode() const;
    std::string generateErrorResponse(int error_code);

    bool isLocalRedirect() const;

    void translateToHttp(std::map<std::string, std::string> &cgi_headers);

    CGIResponse();
    ~CGIResponse();

  private:
    CgiState          m_cgi_state;
    std::string       m_cgi_header_buffer;
    std::string       m_http_response_headers;
    std::vector<char> m_body_buffer;
    bool              m_is_local_redirect;
    int               m_error_code;
};