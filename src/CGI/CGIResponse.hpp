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

    void appendCgiData(const char *data, size_t length);
    bool parseCgiHeaders();
    void appendTerminalChunk();

    std::vector<char> &getBodyBuffer();
    void               consumeBodyChunk(size_t length);
    bool               isBufferFull() const;

    void     setCgiState(CgiState state);
    CgiState getCgiState() const;
    int      getErrorCode() const;
    void     generateErrorResponse(int error_code);
    bool     isLocalRedirect() const;

    std::string translateToHttp(std::map<std::string, std::string> &cgi_headers);

    CGIResponse();
    ~CGIResponse();

  private:
    static const size_t CHUNK_SIZE_LIMIT = 64 * 1024;

    CgiState          m_cgi_state;
    std::string       m_cgi_header_buffer;
    std::vector<char> m_body_buffer;
    bool              m_is_local_redirect;
    int               m_error_code;
};