#include "Router.hpp"

bool string_start_with(const std::string &bigger_string, const std::string &prefix)
{
    if (bigger_string.length() < prefix.length())
    {
        return false;
    }
    return std::equal(prefix.begin(), prefix.end(), bigger_string.begin());
}

std::string join_path(const std::string &root, const std::string &rest)
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

bool path_traverse_is_safe(const std::string &str)
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
