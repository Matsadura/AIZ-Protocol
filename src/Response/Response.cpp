#include "Response.hpp"

Response::Response(const Request &request) :
    m_state(RESPONSE_INIT),
    m_buffer_offset(0),
    m_status_code(request.getErrorCode()), 
    m_request(request)

{
}

Response::~Response(void)
{
}

/**
 * validate the request and prepare the response
 */

void   Response::init_response()
{
    if(m_request.getState() != Request::COMPLETE)
    {
        m_state = ERROR;
        m_status_code = m_request.getErrorCode();
        return;
    }
    else
    {
        struct stat bff = {};
        std::string file = "index.html"; // index.html to be replaced by the actual path provided by the router
        if (stat(file.c_str(), &bff) != 0) 
        {
            m_state = ERROR;
            if(errno == ENOENT)
                m_status_code = NOT_FOUND;
            else if(errno == EACCES)
                m_status_code = FORBIDDEN;
            return;
        }
        m_file_input.open(file.c_str());
        if(!m_file_input)
        {
            m_status_code = INTERNAL_SERVER_ERROR;
            m_state = ERROR;
            return;
        }
        m_state = RESPONSE_SEND_HEADERS;
    }
}

void Response::process(int fd)
{
    switch (m_state) 
    {
        case RESPONSE_INIT:
            //init response*fd
            break;
        case RESPONSE_SEND_HEADERS:
            // sending chunks
            break;
        case RESPONSE_SEND_CHUNKS:
            // sending chunks
            break;
        case ERROR:
            //handle error
            break;
        case RESPONSE_COMPLETE:
            ///
            break;
    }
}
