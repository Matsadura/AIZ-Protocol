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
    return forbidden();
}

RouterResult Router::init_error_result(int http_code)
{
    const std::string &file_path = RouterResource::get_default_page(m_server, http_code);
    return RouterResult(http_code, file_path, RouterResult::FILE_PATH);
}

RouterResult Router::not_found()
{
    return init_error_result(404);
}

RouterResult Router::forbidden()
{
    return init_error_result(403);
}

RouterResult Router::directory_listing(const std::string &dir_path)
{
    return RouterResult(200, generate_directory_listing(dir_path, m_uri), RouterResult::STRING_BUFFER);
}

RouterResult Router::redirection(int http_code, const std::string &rediretion_location)
{
    return RouterResult(http_code, rediretion_location, RouterResult::REDIRECTION);
}

RouterResult Router::handle_get()
{
    std::string disk_path = m_resource.get_disk_path();

    if (!m_resource.exists())
    {
        router_log_helper(m_uri, m_resource.get_location(), disk_path, "File or directory does not exist", 404);
        return not_found();
    }

    if (m_resource.is_directory())
    {
        if (m_uri.empty() || m_uri[m_uri.size() - 1] != '/')
        {
            router_log_helper(m_uri, m_resource.get_location(), m_uri + "/",
                              "Directory; redirecting with trailing slash", 301);
            return redirection(301, m_uri + "/");
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
                return forbidden();
            }
            router_log_helper(m_uri, loc, disk_path, "Directory listing will be performed", 200);
            return directory_listing(disk_path);
        }
        else
        {
            router_log_helper(m_uri, loc, disk_path, "Directory with no index file and listing is off", 403);
            return forbidden();
        }
    }

    if (!is_file_readable(disk_path))
    {
        router_log_helper(m_uri, m_resource.get_location(), disk_path, "File permission denied", 403);
        return forbidden();
    }
    router_log_helper(m_uri, m_resource.get_location(), disk_path, "Will be served", 200);
    return RouterResult(200, disk_path, RouterResult::FILE_PATH);
}
