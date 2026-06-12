#include "Response.hpp"

Response::Response(const Request &request) :
    m_state(RESPONSE_INIT), m_status_code(request.getErrorCode()), m_request(request)

{
}

Response::~Response(void)
{
}
