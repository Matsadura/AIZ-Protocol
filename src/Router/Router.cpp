#include "Router.hpp"

/**
 * Tells if @resource requested by the user matches the @location_path at the config file
 * so "/img" matches "/img" and "/img/x.png", but NOT "/images/x.png"
 *
 * Return: true meaning @loc_path is a possible match for @resource, or false meaning the opposite in case you still
 * reading scream "THIS IS OUR REVIVALE"
 */
bool router_location_matcher(const std::string &resource, const std::string &loc_path)
{
    if (!string_start_with(resource, loc_path))
        return false;
    if (resource.size() == loc_path.size())
        return true;
    if (loc_path[loc_path.size() - 1] == '/')
        return true;
    return resource[loc_path.size()] == '/';
}

/**
 * Get the best matching config file location for the @resource
 *
 * Return: pointer for the config file location matching the resource or NULL
 */
const s_Location *router_get_best_matched_location(const s_Server &server, const std::string &resource)
{
    const s_Location *location = NULL;

    for (size_t j = 0; j < server.locations.size(); ++j)
    {
        if (router_location_matcher(resource, server.locations[j].path))
        {
            if (!location || server.locations[j].path.size() > location->path.size())
            {
                location = &server.locations[j];
            }
        }
    }
    return location;
}

/**
 * Simple logging utility for debugging, sometime in the near future you'll ask what WTF that just happend this will
 * help!
 */
void router_log_helper(const std::string &resource, const s_Location *loc, const std::string &file_path,
                       const std::string &msg, int http_code)
{
    LOG_DEBUG("ROUTER") << "[URI=" << resource << "]";
    if (loc)
        std::cout << " [config_loc=" << loc->path << "]";
    std::cout << " [";
    if (!file_path.empty())
        std::cout << file_path;
    std::cout << ": " << msg << "]" << " (status_code=" << http_code << ")\n";
}

bool is_file_regular(const std::string &path)
{
    struct stat file_info = {};

    return stat(path.c_str(), &file_info) == 0 && S_ISREG(file_info.st_mode);
}

bool is_file_executable(const std::string &filepath)
{
    return access(filepath.c_str(), X_OK) == 0;
}

/**
 * Return: path of the default page for the @status_code or empty string meaning use your default, you don't have a
 * default? that you problem!
 *
 * NOTE: Each server could have muliple default paths foreach error page, this will get them for you
 */
std::string get_default_page(const s_Server &server, int status_code)
{
    std::map<int, std::string>::const_iterator it = server.error_page.find(status_code);
    if (it == server.error_page.end())
        return "";
    std::string page_path = it->second;
    if (is_file_regular(page_path))
    {
        return page_path;
    }
    return "";
}

bool is_file_readable(const std::string &filepath)
{
    return access(filepath.c_str(), R_OK) == 0;
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

CgiMetaData router_request_is_cgi(const s_Server &server, const Request &req)
{
    const s_Location *location = router_get_best_matched_location(server, req.getURI());

    if (!location || location->CGIhandlers.empty())
        return CgiMetaData();

    std::string sys_path = join_path(location->root, req.getURI().substr(location->path.size()));
    std::string script_path;
    std::string path_info;

    for (std::size_t i = 0; i <= sys_path.size(); i++)
    {
        if (i == sys_path.size() || sys_path[i] == '/')
        {
            std::string current_path = sys_path.substr(0, i);
            if (is_file_regular(current_path))
            {
                script_path = current_path;
                if (i < sys_path.size())
                {
                    path_info = sys_path.substr(i);
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

RouterResult router_get_resource(const s_Server &server, const std::string &resource)
{
    const s_Location *location  = router_get_best_matched_location(server, resource);
    struct stat       file_info = {};

    // TODO: Handle request type GET/HEAD...
    // TODO: Handle relative paths

    if (!location)
    {
        router_log_helper(resource, location, "", "no matching location", 404);
        return RouterResult(404, get_default_page(server, 404), RouterResult::ERROR_PAGE);
    }

    if (location->redirect_code != 0)
    {
        std::string redirect_uri(resource);
        redirect_uri.erase(0, location->path.size());
        redirect_uri = join_path(location->redirect_path, redirect_uri);
        if (redirect_uri[0] != '/')
        {
            redirect_uri = "/" + redirect_uri;
        }
        router_log_helper(resource, location, redirect_uri, "Redirection location", location->redirect_code);
        return RouterResult(location->redirect_code, redirect_uri, RouterResult::REDIRECTION);
    }

    if (location->root.empty())
    {
        router_log_helper(resource, location, "", "Root directory is not configured", 404);
        return RouterResult(404, get_default_page(server, 404), RouterResult::ERROR_PAGE);
    }

    std::string sys_path(resource);
    sys_path.erase(0, location->path.size());
    sys_path = join_path(location->root, sys_path);

    // TODO: CGI routing goes here?

    if (stat(sys_path.c_str(), &file_info) != 0)
    {
        router_log_helper(resource, location, sys_path, "File or directory does not exist", 404);
        return RouterResult(404, get_default_page(server, 404), RouterResult::ERROR_PAGE);
    }
    if (S_ISDIR(file_info.st_mode))
    {
        if (resource.empty() || resource[resource.size() - 1] != '/')
        {
            router_log_helper(resource, location, resource + "/", "Directory; redirecting with trailing slash", 301);
            return RouterResult(301, resource + "/", RouterResult::REDIRECTION);
        }

        std::string indexPath = join_path(sys_path, location->index);
        if (!location->index.empty() && is_file_regular(indexPath))
        {
            sys_path = indexPath;
        }
        else if (location->autoindex)
        {
            if (!is_file_readable(sys_path))
            {
                router_log_helper(resource, location, sys_path, "Directory is unreadable for listing", 403);
                return RouterResult(403, get_default_page(server, 403), RouterResult::ERROR_PAGE);
            }
            router_log_helper(resource, location, sys_path, "Directory listing will be performed", 200);
            return RouterResult(200, generate_directory_listing(sys_path, resource), RouterResult::STRING_BUFFER);
        }
        else
        {
            router_log_helper(resource, location, sys_path, "Directory with no index file and listing is off", 403);
            return RouterResult(403, get_default_page(server, 403), RouterResult::ERROR_PAGE);
        }
    }

    if (!is_file_readable(sys_path))
    {
        router_log_helper(resource, location, sys_path, "File permission denied", 403);
        return RouterResult(403, get_default_page(server, 403), RouterResult::ERROR_PAGE);
    }
    router_log_helper(resource, location, sys_path, "Will be served", 200);
    return RouterResult(200, sys_path, RouterResult::FILE_PATH);
}
