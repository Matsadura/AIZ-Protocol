#include "Router.hpp"

bool string_start_with(const std::string &bigger_string, const std::string &prefix)
{
    if (bigger_string.length() < prefix.length())
        return false;
    return std::equal(prefix.begin(), prefix.end(), bigger_string.begin());
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
