#include "Response.hpp"
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstddef>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>

Response::Response(const Request &request) :
    m_state(RESPONSE_INIT),
    m_buffer_offset(0),
    m_status_code(200), 
    m_request(request),
    m_content_length(0)

{
}

Response::~Response(void)
{
}

/**
 * validate the request and prepare the response
 */
void    Response::handleErrors()
{
    m_status_code = m_request.getErrorCode();
    m_state = RESPONSE_SEND_HEADERS;
    std::stringstream ss;
    ss << "<h1>" << m_status_code << getStatusMessage(m_status_code) << "</h1>";
    m_body_content = ss.str();
}


void   Response::init_response()
{
    if(m_request.getState() != Request::COMPLETE)
    {

        return;
    }
    std::string file = "index.html"; // index.html to be replaced by the actual path provided by the router
    std::string method = m_request.getMethod();
    
    if(method == "GET")
    {
        struct stat bff = {};
        if (stat(file.c_str(), &bff) != 0) 
        {
            if(errno == ENOENT)
                m_status_code = NOT_FOUND;
            else if(errno == EACCES)
                m_status_code = FORBIDDEN;
        }
        else 
        {
            m_file_input.open(file.c_str(), std::ios::binary);
            if(!m_file_input)
                m_status_code = INTERNAL_SERVER_ERROR;
            else
            {
                std::stringstream ss;
                ss << m_file_input.rdbuf();
                m_body_content = ss.str();
                m_file_input.close();
                m_status_code = 200;
            }
        }
    }
    else if(method == "DELETE")
    {
        if(unlink(file.c_str()) == 0)
        {
            m_status_code = 204;
            m_body_content = "";
        }
        else {
            if(errno == EACCES) m_status_code = FORBIDDEN;
            else if(errno == ENOENT) m_status_code = NOT_FOUND;
            else m_status_code = INTERNAL_SERVER_ERROR;
        }
    }
    else if(method == "POST")
    {
        std::ofstream outfile(file.c_str(), std::ios::binary);
        if(!outfile)
            m_status_code = INTERNAL_SERVER_ERROR;
        else
        {
            std::vector<char> vec_body = m_request.getBody();
            if(!vec_body.empty())
                outfile.write(&vec_body[0], static_cast<std::streamsize>(vec_body.size()));
            outfile.close();
            m_status_code = 201;
            m_body_content = "<html><body><h1>File Uploaded Successfully</h1></body></html>";
        }
    }
    if(!m_body_content.empty())
        m_content_length = m_body_content.size();
    m_state = RESPONSE_SEND_HEADERS;
}

std::string Response::getStatusMessage(int code)
{
    switch (code)
    {
        case 200: return "OK";
        case 201: return "CREATED";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 431: return "Header Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        default:  return "Internal Server Error";
    }
}

/**
 * header handler
 * @fd: socket fd
 */
void   Response::header_handler()
{
    std::stringstream ss;
    ss << "HTTP/1.0 " << m_status_code << " " << getStatusMessage(m_status_code) << CRLF;
    ss << "Server: webserv/1.0" << CRLF;
    ss << "Content-Type: text/html" << CRLF;
    ss << "Transfer-Encoding: chunked" << CRLF;
    ss << CRLF;

    m_response_buffer = ss.str();
    m_state = RESPONSE_SEND_CHUNKS;
}

std::string Response::toHex(size_t size)
{
    std::stringstream ss;

    ss << std::hex << size;
    return ss.str();
}

void    Response::chunks_handler()
{
    // success
    if(m_status_code != 200)
    {
        // if the Error page exists
        // m_file_input = map[errorcode];
        // if the Error page is nowhere to be found
            //fallback_html
        std::stringstream fallback_html;
        fallback_html << "<html><body><center><h1>" << m_status_code << " " << getStatusMessage(m_status_code)
           << "</h1></center></body></html>";
        std::string fallback_html_str = fallback_html.str();
        std::stringstream ss;
        ss << toHex(fallback_html_str.size()) << CRLF << fallback_html_str << CRLF;
        m_response_buffer = ss.str();
        m_state = RESPONSE_COMPLETE;
        return;
    }

}



void Response::process()
{
    switch (m_state) 
    {
        case RESPONSE_INIT:
            init_response();
            break;
        case RESPONSE_SEND_HEADERS:
            header_handler();
            break;
        case RESPONSE_SEND_CHUNKS:
            chunks_handler();
            break;
        case RESPONSE_COMPLETE:
            ///
            break;
    }
}
