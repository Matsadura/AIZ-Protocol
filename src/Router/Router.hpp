#include "../Request/Request.hpp"
#include "../config_file_parser/parser/configfile.hpp"
#include "../core/Common.h"
#include "RouterResource.hpp"
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Returns HTML listing of the @dir_path, the @uri_path is the resource uri requested by the user
 */
std::string generate_directory_listing(const std::string &dir_path, const std::string &uri_path);

class Router
{
    const s_Server &m_server;
    // const Request    &m_request;
    const std::string m_uri;
    const std::string m_method;
    RouterResource    m_resource;

  public:
    Router(const s_Server &server, const std::string &uri, const std::string &method);
    RouterResult get_result();

    // helper to init RouterResult
    RouterResult init_error_result(int http_code);
    RouterResult http_not_found();
    RouterResult http_forbidden();
    RouterResult http_redirection(int http_code, const std::string &location);
    RouterResult http_directory_listing(const std::string &dir_path);
    RouterResult http_conflict();
    RouterResult http_no_content();
    RouterResult http_method_not_allowed();

    RouterResult handle_get();
    RouterResult handle_delete();
};
