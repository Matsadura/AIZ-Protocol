#include "RouterResource.hpp"

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

RouterResource::RouterResource(const s_Server &server, const std::string &uri, const std::string &method) :
    m_location(NULL),
    m_uri(uri),
    m_method(method),
    m_router_result(404, get_default_page(server, 404), RouterResult::FILE_PATH),
    m_has_early_response(false)
{
    m_location = get_best_matched_location(server, m_uri);

    if (!m_location)
    {
        router_log_helper(m_uri, m_location, "", "no matching location", 404);
        set_early_response(404, server);
        return;
    }

    if (m_location->redirect_code != 0)
    {
        std::string redirect_uri(m_uri);
        redirect_uri.erase(0, m_location->path.size());
        redirect_uri = join_path(m_location->redirect_path, redirect_uri);
        if (redirect_uri[0] != '/')
        {
            redirect_uri = "/" + redirect_uri;
        }
        router_log_helper(m_uri, m_location, redirect_uri, "Redirection location", m_location->redirect_code);
        set_early_redirect_response(m_location->redirect_code, redirect_uri);
        return;
    }

    if (!location_allowed_method(*m_location, m_method))
    {
        router_log_helper(m_uri, m_location, m_method, "Method not allowed", 405);
        set_early_response(405, server);
        return;
    }

    m_subpath = m_uri;
    m_subpath.erase(0, m_location->path.size());
    if (!path_traverse_is_safe(m_subpath)) // Prevent path traversing like: ../././../..
    {
        router_log_helper(m_uri, m_location, m_subpath, "Attempting to escapee root", 400);
        set_early_response(400, server);
        return;
    }
}

const s_Location *RouterResource::get_best_matched_location(const s_Server &server, const std::string &resource)
{
    const s_Location *location = NULL;

    for (size_t j = 0; j < server.locations.size(); ++j)
    {
        if (location_matcher(resource, server.locations[j].path))
        {
            if (!location || server.locations[j].path.size() > location->path.size())
            {
                location = &server.locations[j];
            }
        }
    }
    return location;
}

bool RouterResource::location_matcher(const std::string &resource, const std::string &loc_path)
{
    if (!string_starts_with(resource, loc_path))
    {
        return false;
    }
    if (resource.size() == loc_path.size())
    {
        return true;
    }
    if (loc_path[loc_path.size() - 1] == '/')
    {
        return true;
    }
    return resource[loc_path.size()] == '/';
}

void RouterResource::set_early_response(int http_code, const s_Server &server)
{
    m_has_early_response = true;
    m_router_result      = RouterResult(http_code, get_default_page(server, http_code), RouterResult::FILE_PATH);
}

void RouterResource::set_early_redirect_response(int http_code, const std::string &location)
{
    m_has_early_response = true;
    m_router_result      = RouterResult(http_code, location, RouterResult::REDIRECTION);
}

std::string RouterResource::get_default_page(const s_Server &server, int status_code)
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

std::string RouterResource::join_path(const std::string &root, const std::string &rest)
{
    if (root.empty() || rest.empty())
    {
        return root + rest;
    }

    bool rootSlash = root[root.size() - 1] == '/';
    bool restSlash = rest[0] == '/';
    if (rootSlash && restSlash)
    {
        return root + rest.substr(1);
    }
    if (!rootSlash && !restSlash)
    {
        return root + "/" + rest;
    }
    return root + rest;
}

bool RouterResource::location_allowed_method(const s_Location &location, const std::string &method)
{
    const std::vector<std::string> &methods = location.methods;

    return method == "GET" || std::find(methods.begin(), methods.end(), method) != methods.end();
}

static bool cstr_cmp_by_size(const char *s1, const char *s2, std::size_t size)
{
    for (std::size_t i = 0; i < size; i++)
    {
        if (s1[i] != s2[i])
        {
            return false;
        }
    }
    return true;
}

bool RouterResource::path_traverse_is_safe(const std::string &str)
{
    const char       *path  = str.data();
    const std::size_t len   = str.size();
    int               level = 0;
    size_t            i     = 0;

    while (i < len)
    {
        while (i < len && path[i] == '/')
        {
            i++;
        }
        if (i >= len)
        {
            break;
        }

        std::size_t seg_len = 0;
        while (i < len && path[i] != '/')
        {
            i++;
            seg_len++;
        }

        // TODO: Can "...." and "..."  cause a problem?
        if (seg_len == 2 && cstr_cmp_by_size(&path[i - seg_len], "..", 2))
        {
            level--;
            if (level < 0)
            {
                return false;
            }
        }
        else if (seg_len == 1 && path[i - seg_len] == '.')
        {
            continue;
        }
        else if (seg_len > 0)
        {
            level++;
        }
    }

    return level >= 0;
}

bool RouterResource::has_early_response()
{
    return m_has_early_response;
}

RouterResult RouterResource::get_early_router_result()
{
    if (!m_has_early_response)
    {
        LOG_ERROR("RouterResource") << "[uri=" << m_uri << "] Attempting to get none existing early router result!\n";
    }
    return m_router_result;
}

const s_Location *RouterResource::get_location()
{
    return m_location;
}

std::string RouterResource::get_req_path()
{
    return m_uri;
}

std::string RouterResource::get_subpath()
{
    return m_subpath;
}

bool RouterResource::validate_root_path()
{
    if (m_location->root.empty())
    {
        router_log_helper(m_uri, m_location, "", "Root directory is not configured", 404);
        return false;
    }

    if (!is_directory_path(m_location->root))
    {
        router_log_helper(m_uri, m_location, m_location->root, "Root directory does not exist or not a directory", 404);
        return false;
    }
    return true;
}

bool RouterResource::validate_upload_store_path()
{
    if (m_location->uploadStore.empty())
    {
        router_log_helper(m_uri, m_location, "", "Upload store is not configured", 404);
        return false;
    }

    if (!is_directory_path(m_location->uploadStore))
    {
        router_log_helper(m_uri, m_location, m_location->uploadStore, "Upload store does not exist or not a directory",
                          404);
        return false;
    }
    return true;
}
