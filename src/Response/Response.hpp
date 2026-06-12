#pragma once

#include "../Request/Request.hpp"
#include <fstream>

#define NOT_FOUND 404

class Response
{
    public:
        enum ResponseState
        {
            RESPONSE_INIT,
            RESPONSE_SEND_HEADERS,
            RESPONSE_SEND_CHUNKS,
            RESPONSE_COMPLETE
        };
        Response(const Request &request);
        ~Response(void);
    private:
        ResponseState m_state;
        int         m_status_code;
        std::ifstream m_file_input;
        Request m_request;
};