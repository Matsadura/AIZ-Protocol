#include "../Request/Request.hpp"
#include "../config_file_parser/parser/configfile.hpp"
#include "../core/Common.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Use @m_data_type to decide what to do, because @m_data can be multiple things
 */
class RouterResult
{
  public:
    enum Type
    {
        STRING_BUFFER, // @m_data is the content that will be served
        FILE_PATH,     // @m_data can be a valid file_path or empty, if its empty means use your default
        REDIRECTION,   // @m_data is the location for the redirection
    };

    int         m_http_code;
    std::string m_data;
    Type        m_data_type;

    RouterResult(int http_code, const std::string &data, Type data_type) :
        m_http_code(http_code), m_data(data), m_data_type(data_type)
    {
    }

    bool operator==(const RouterResult &other)
    {
        return m_http_code == other.m_http_code && m_data == other.m_data && m_data_type == other.m_data_type;
    }
};

/**
 * Join two path segments
 *
 * The return value is the concatenation of two paths
 * I only use `/` so this screams NOT CROSS-PLATFORM, mybe something to fix later?
 */
std::string join_path(const std::string &root, const std::string &rest);

/**
 * Returns: false if path  will traverse above the root directory, true if its not
 */
bool path_traverse_is_safe(const std::string &str);

/**
 * Returns a Boolean stating whether a string starts with the specified prefix
 */
bool string_start_with(const std::string &bigger_string, const std::string &prefix);

/**
 * Returns HTML listing of the @dir_path, the @uri_path is the resource uri requested by the user
 */
std::string generate_directory_listing(const std::string &dir_path, const std::string &uri_path);

/**
 * Return: RouterResult object, refer for its docs string to know how to use it
 *
 * @resource: is the uri requested by the user
 * @server: the one that received the connection
 */
RouterResult router_get_resource(const s_Server &server, const std::string &resource);
std::string  get_default_page(const s_Server &server, int status_code);
