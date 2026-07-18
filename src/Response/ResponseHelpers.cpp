#include "Response.hpp"

std::string Response::getStatusMessage(int code)
{
    switch (code)
    {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 204:
            return "No Content";
        case 400:
            return "Bad Request";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 413:
            return "Payload Too Large";
        case 431:
            return "Header Too Large";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Found";
        case 303:
            return "See Other";
        case 307:
            return "Temporary Redirect";
        case 308:
            return "Permanent Redirect";
        default:
            return "Internal Server Error";
    }
}


size_t get_file_length(std::string &file)
{
    struct stat st = {};
    if (stat(file.c_str(), &st) == 0)
        return st.st_size;
    return 0;
}


std::string Response::get_content_type(const std::string &filepath)
{
    if(m_router.m_data_type == RouterResult::STRING_BUFFER)
        return "text/html";
    else if(m_router.m_data_type == RouterResult::REDIRECTION)
        return "";
    size_t dot_pos = filepath.find_last_of('.');
    if (dot_pos == std::string::npos)
    {
        return "application/octet-stream";
    }

    std::string ext = filepath.substr(dot_pos + 1);
    if (ext == "html" || ext == "htm")
        return "text/html";
    if (ext == "css")
        return "text/css";
    if (ext == "js")
        return "application/javascript";
    if (ext == "png")
        return "image/png";
    if (ext == "jpg" || ext == "jpeg")
        return "image/jpeg";
    if (ext == "gif")
        return "image/gif";
    if (ext == "json")
        return "application/json";
    if (ext == "txt")
        return "text/plain";
    return "application/octet-stream";
}
