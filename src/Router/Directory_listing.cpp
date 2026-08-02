#include "Router.hpp"
#include "RouterResource.hpp"
#include <iomanip>

bool comp_name(const std::pair<bool, std::string> &a, const std::pair<bool, std::string> &b)
{
    if (a.first != b.first)
        return a.first > b.first; // dir always comes first!
    return a.second < b.second;
}

std::string url_encode(const std::string &value)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex << std::uppercase;

    for (size_t i = 0; i < value.length(); ++i)
    {
        unsigned char c = value[i];

        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
        }
        else
        {
            escaped << '%' << std::setw(2) << int(c);
        }
    }
    return escaped.str();
}

std::string html_escape_special_chars(const std::string &in)
{
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i)
    {
        switch (in[i])
        {
            case '&':
                out += "&amp;";
                break;
            case '<':
                out += "&lt;";
                break;
            case '>':
                out += "&gt;";
                break;
            case '\"':
                out += "&quot;";
                break;
            case '\'':
                out += "&#39;";
                break;
            default:
                out += in[i];
                break;
        }
    }
    return out;
}

std::string generate_directory_listing(const std::string &dir_path, const std::string &uri_path)
{
    std::stringstream                          output;
    DIR                                       *d;
    struct dirent                             *dir;
    std::vector<std::pair<bool, std::string> > entries;

    d = opendir(dir_path.c_str());
    if (!d)
        return "";

    while ((dir = readdir(d)) != NULL)
    {
        std::string name = dir->d_name;
        if (name == ".")
            continue;

        struct stat sb     = {};
        bool        is_dir = false;
        if (stat(RouterResource::join_path(dir_path, name).c_str(), &sb) == 0)
            is_dir = S_ISDIR(sb.st_mode);

        entries.push_back(std::make_pair(is_dir, name));
    }
    closedir(d);

    std::sort(entries.begin(), entries.end(), comp_name);

    output << "<html><head><title>Index of " << uri_path << "</title></head><body>\n";
    output << "<h1>Index of " << uri_path << "</h1><hr><pre>\n";

    for (size_t i = 0; i < entries.size(); ++i)
    {
        bool               is_dir = entries[i].first;
        const std::string &name   = entries[i].second;
        std::string        suffix = is_dir ? "/" : "";

        output << "<a href=\"" << url_encode(name) << suffix << "\">" << html_escape_special_chars(name) << suffix
               << "</a>\n";
    }

    output << "</pre><hr></body></html>\n";
    return output.str();
}
