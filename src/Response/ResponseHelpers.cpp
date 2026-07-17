#include "Response.hpp"

void Response::generateErrorBody()
{
    std::stringstream ss;
    ss << "<h1>" << m_status_code << " " << getStatusMessage(m_status_code) << "</h1>";
    m_body_content = ss.str();
    m_content_length = m_body_content.size();
}

// void Response::init_GET()
// {
//     if(m_router.m_data_type == RouterResult::FILE_PATH)
//     {
//         std::ifstream infile(m_router.m_data.c_str(), std::ios::binary);
//         if(!infile.is_open())
//         {
//             generateErrorBody();
//             return;
//         }
//         std::stringstream buffer;
//         buffer << infile.rdbuf();
//         m_body_content = buffer.str();
//         m_content_type = get_content_type(m_router.m_data);
//     }
//     else if(m_router.m_data_type == RouterResult::STRING_BUFFER)
//     {
//         m_body_content = m_router.m_data;
//         m_content_type = "text/html";
//     }
//     m_content_length = m_body_content.size();
// }

// void Response::init_DELETE()
// {
//     if (unlink(file_path.c_str()) == 0)
//     {
//         m_status_code  = 204;
//         m_body_content = "";
//     }
//     else
//     {
//         if (errno == EACCES)
//             m_status_code = FORBIDDEN;
//         else if (errno == ENOENT)
//             m_status_code = NOT_FOUND;
//         else
//             m_status_code = INTERNAL_SERVER_ERROR;
//         generateErrorBody();
//         return;
//     }
// }

// void Response::init_POST()
// {
//     std::ofstream outfile(file_path.c_str(), std::ios::binary);
//     if (!outfile)
//     {
//         m_status_code = INTERNAL_SERVER_ERROR;
//         generateErrorBody();
//         return;
//     }
//     else
//     {
//         std::vector<char> vec_body = m_request.getBody();
//         if (!vec_body.empty())
//             outfile.write(&vec_body[0], static_cast<std::streamsize>(vec_body.size()));
//         outfile.close();
//         m_status_code  = 201;
//         m_body_content = "<html><body><h1>File Uploaded Successfully</h1></body></html>";
//     }
// }

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
