#include "Router.hpp"

Router::Router(const s_Server &server, const std::string &uri, const std::string &method) :
    m_server(server), m_uri(uri), m_method(method), m_resource(server, uri, method)
{
}

RouterResult Router::get_result()
{
    if (m_resource.has_early_response())
    {
        return m_resource.get_early_router_result();
    }
    if (m_method == "GET")
    {
        return handle_get();
    }
    if (m_method == "DELETE")
    {
        return handle_delete();
    }
    return http_method_not_allowed();
}

RouterResult Router::init_error_result(int http_code)
{
    const std::string &file_path = RouterResource::get_default_page(m_server, http_code);
    return RouterResult(http_code, file_path, RouterResult::FILE_PATH);
}

RouterResult Router::http_directory_listing(const std::string &dir_path)
{
    return RouterResult(200, generate_directory_listing(dir_path, m_uri), RouterResult::STRING_BUFFER);
}

RouterResult Router::http_no_content()
{
    return init_error_result(204);
}

RouterResult Router::http_redirection(int http_code, const std::string &rediretion_location)
{
    return RouterResult(http_code, rediretion_location, RouterResult::REDIRECTION);
}

RouterResult Router::http_forbidden()
{
    return init_error_result(403);
}

RouterResult Router::http_not_found()
{
    return init_error_result(404);
}

RouterResult Router::http_method_not_allowed()
{
    return init_error_result(405);
}

RouterResult Router::http_conflict()
{
    return init_error_result(409);
}

RouterResult Router::handle_get()
{
    std::string disk_path = m_resource.get_disk_path();

    if (!m_resource.exists())
    {
        router_log_helper(m_uri, m_resource.get_location(), disk_path, "File or directory does not exist", 404);
        return http_not_found();
    }

    if (m_resource.is_directory())
    {
        if (m_uri.empty() || m_uri[m_uri.size() - 1] != '/')
        {
            router_log_helper(m_uri, m_resource.get_location(), m_uri + "/",
                              "Directory; redirecting with trailing slash", 301);
            return http_redirection(301, m_uri + "/");
        }

        const s_Location *loc = m_resource.get_location();

        std::string indexPath = RouterResource::join_path(disk_path, loc->index);
        if (!loc->index.empty() && is_file_regular(indexPath))
        {
            disk_path = indexPath;
        }
        else if (loc->autoindex)
        {
            if (!is_file_readable(disk_path))
            {
                router_log_helper(m_uri, loc, disk_path, "Directory is unreadable for listing", 403);
                return http_forbidden();
            }
            router_log_helper(m_uri, loc, disk_path, "Directory listing will be performed", 200);
            return http_directory_listing(disk_path);
        }
        else
        {
            router_log_helper(m_uri, loc, disk_path, "Directory with no index file and listing is off", 403);
            return http_forbidden();
        }
    }

    if (!is_file_readable(disk_path))
    {
        router_log_helper(m_uri, m_resource.get_location(), disk_path, "File permission denied", 403);
        return http_forbidden();
    }
    router_log_helper(m_uri, m_resource.get_location(), disk_path, "Will be served", 200);
    return RouterResult(200, disk_path, RouterResult::FILE_PATH);
}

RouterResult Router::handle_delete()
{
    std::string       disk_path = m_resource.get_disk_path();
    const s_Location *loc       = m_resource.get_location();

    if (!m_resource.exists())
    {
        router_log_helper(m_uri, loc, disk_path, "File or directory does not exist", 404);
        return http_not_found();
    }

    if (m_resource.is_directory() && (m_uri.empty() || m_uri[m_uri.size() - 1] != '/'))
    {
        router_log_helper(m_uri, loc, m_uri + "/", "Directory; cannot delete without trailing slash", 409);
        return http_conflict();
    }

    if (std::remove(disk_path.c_str()) != 0)
    {
        int         err     = errno;
        const char *err_msg = std::strerror(err);

        if (err == ENOTEMPTY || err == EEXIST)
        {
            router_log_helper(m_uri, loc, disk_path, err_msg, 409);
            return http_conflict();
        }

        router_log_helper(m_uri, loc, disk_path, err_msg, 403);
        return http_forbidden();
    }
    else
    {
        router_log_helper(m_uri, loc, disk_path, "Has been deleted", 204);
        return http_no_content();
    }
}

RouterResult Router::handle_post()
{
    std::string       disk_path = m_resource.get_disk_path();
    const s_Location *loc       = m_resource.get_location();

    if (m_resource.exists())
    {
        if (m_resource.is_directory())
        {
            router_log_helper(m_uri, loc, disk_path, "Directory posting is not allowed", 403);
            return http_forbidden();
        }

        if (access(disk_path.c_str(), W_OK) != 0)
        {
            router_log_helper(m_uri, loc, disk_path, "File is not writable", 403);
            return http_forbidden();
        }
    }
    else
    {
        std::string parent_dir;
        size_t      last_slash = disk_path.find_last_of('/');

        if (last_slash == std::string::npos)
        {
            parent_dir = ".";
        }
        else if (last_slash == 0)
        {
            parent_dir = "/";
        }
        else
        {
            parent_dir = disk_path.substr(0, last_slash);
        }

        if (access(parent_dir.c_str(), W_OK | X_OK) != 0)
        {
            router_log_helper(m_uri, loc, parent_dir, "Parent directory is not writable/accessible", 403);
            return http_forbidden();
        }
    }

    router_log_helper(m_uri, loc, disk_path, "Path is writable for POST", 200);
    return RouterResult(200, disk_path, RouterResult::FILE_PATH);
}

struct CgiMetaData
{
    bool        is_cgi;
    std::string script_path;
    std::string path_info;
    std::string interpreter_path;

    CgiMetaData() : is_cgi(false)
    {
    }

    CgiMetaData(bool is_cgi_val, const std::string &script, const std::string &info, const std::string &interpreter) :
        is_cgi(is_cgi_val), script_path(script), path_info(info), interpreter_path(interpreter)
    {
    }
};

CgiMetaData is_cgi_request(const s_Server &server, const Request &req)
{
    const s_Location *location = RouterResource::get_best_matched_location(server, req.getPath());

    if (!location || location->CGIhandlers.empty() || location->root.empty())
    {
        return CgiMetaData();
    }

    std::string disk_path = RouterResource::join_path(location->root, req.getPath().substr(location->path.size()));
    std::string script_path;
    std::string path_info;

    for (std::size_t i = 0; i <= disk_path.size(); i++)
    {
        if (i == disk_path.size() || disk_path[i] == '/')
        {
            std::string current_path = disk_path.substr(0, i);
            // TODO: Not save! location->root will be counted going above root will happen anyway
            if (!RouterResource::path_traverse_is_safe(current_path))
            {
                return CgiMetaData();
            }
            if (is_file_regular(current_path))
            {
                script_path = current_path;
                if (i < disk_path.size())
                {
                    path_info = disk_path.substr(i);
                }
                break;
            }
        }
    }

    if (script_path.empty())
    {
        return CgiMetaData();
    }

    std::size_t dot_pos   = script_path.find_last_of('.');
    std::size_t slash_pos = script_path.find_last_of('/');

    if (dot_pos != std::string::npos && (slash_pos == std::string::npos || dot_pos > slash_pos))
    {
        std::string                                        ext = script_path.substr(dot_pos);
        std::map<std::string, std::string>::const_iterator it  = location->CGIhandlers.find(ext);

        if (it != location->CGIhandlers.end())
        {
            return CgiMetaData(true, script_path, path_info, it->second);
        }
    }

    return CgiMetaData();
}
