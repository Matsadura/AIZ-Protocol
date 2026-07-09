#include "../config_file_parser/parser/configfile.hpp"
#include <iostream>
#include <sys/stat.h>

bool string_start_with(const std::string &bigger_string, const std::string &prefix)
{
    if (bigger_string.length() < prefix.length())
        return false;
    return std::equal(prefix.begin(), prefix.end(), bigger_string.begin());
}

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

std::string join_path(const std::string &root, const std::string &rest)
{
    bool rootSlash = !root.empty() && root[root.size() - 1] == '/';
    bool restSlash = !rest.empty() && rest[0] == '/';
    if (rootSlash && restSlash)
        return root + rest.substr(1);
    if (!rootSlash && !restSlash)
        return root + "/" + rest;
    return root + rest;
}

void print_location(const s_Location &loc)
{
    std::cout << "[Location] Path: " << (loc.path.empty() ? "/" : loc.path) << "\n";
    std::cout << "        |-- Root:         " << loc.root << "\n";
    std::cout << "        |-- Index:        " << loc.index << "\n";
    std::cout << "        |-- Max Body:     " << loc.max_body_size << " bytes\n";
    std::cout << "        |-- Autoindex:    " << (loc.autoindex ? "on" : "off") << "\n";

    std::cout << "        |-- Methods:      ";
    for (size_t i = 0; i < loc.methods.size(); ++i)
        std::cout << loc.methods[i] << (i == loc.methods.size() - 1 ? "" : ", ");
    std::cout << "\n";

    std::cout << "        |-- Redirection:  " << loc.redirect_code << " -> " << loc.redirect_path << "\n";

    if (!loc.CGIhandlers.empty())
    {
        std::cout << "        |-- CGI Handlers:\n";
        std::map<std::string, std::string>::const_iterator it;
        for (it = loc.CGIhandlers.begin(); it != loc.CGIhandlers.end(); ++it)
            std::cout << "            " << it->first << " -> " << it->second << "\n";
    }

    std::cout << "        |-- Upload Store: " << loc.uploadStore << "\n";
    std::cout << "        --------------------------------------\n";
}

void router_get_resource(const s_Server &server, const std::string &resource)
{

    std::cout << "Resource URI = " << resource << "\n";

    const s_Location *location = NULL;
    struct stat       sb       = {};

    for (size_t j = 0; j < server.locations.size(); ++j)
    {
        std::cout << "Path = " << server.locations[j].path << "\n";
        std::cout << "\"" << resource << "\" starts with " << server.locations[j].path << " => "
                  << router_location_matcher(resource, server.locations[j].path) << "\n\n";
        if (router_location_matcher(resource, server.locations[j].path))
        {
            if (!location || server.locations[j].path.size() > location->path.size())
            {
                location = &server.locations[j];
            }
        }
    }

    if (!location)
    {
        std::cout << "Response: 404\n";
        return;
    }

    print_location(*location);

    if (location->redirect_code != 0)
    {
        std::cout << "Response: " << location->redirect_code << " -> " << location->redirect_path << "\n";
        return;
    }

    std::cout << "Target location = " << location->path << "\n";

    std::string sys_path(resource);
    sys_path.erase(0, location->path.size());
    sys_path = join_path(location->root, sys_path);

    // TODO: CGI routing goes here?

    if (stat(sys_path.c_str(), &sb) != 0)
    {
        std::cout << "Response: 404\n";
        return;
    }
    if (S_ISDIR(sb.st_mode))
    {
        if (resource.empty() || resource[resource.size() - 1] != '/')
        {
            std::cout << "Response: 301 -> " << resource << "/\n"; // the redirect from our earlier diagram
            return;
        }
        std::string indexPath = join_path(sys_path, location->index);
        if (!location->index.empty() && stat(indexPath.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
        {
            sys_path = indexPath;
        }
        else if (location->autoindex)
        {
            std::cout << "Response: 200 (autoindex listing)\n";
            return;
        }
        else
        {
            std::cout << "Response: 403\n";
            return;
        }
    }

    std::cout << "Resolved Path = " << sys_path << "\n";
}

int main()
{
    std::string file_name = "configfile/test.conf";
    ConfigFile  config(file_name);

    s_Server server = config.getConfig()[0];

    router_get_resource(server, "/uploads/target.file");

    std::cout << "Works\n";
}
