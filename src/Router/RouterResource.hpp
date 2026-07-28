#ifndef ROUTER_RESOURCE_H
#define ROUTER_RESOURCE_H

#include "../Request/Request.hpp"
#include "../config_file_parser/parser/configfile.hpp"
#include "../core/Common.h"
#include "RouterResult.hpp"
#include <sys/stat.h>
#include <unistd.h>

class RouterResource
{
  private:
    const s_Location *m_location;
    std::string       m_uri;
    std::string       m_method;
    std::string       m_disk_path;
    struct stat       m_file_info;
    bool              m_exists;
    bool              m_is_directory;
    bool              m_readable;

    RouterResult m_router_result;
    bool         m_has_early_response;

    void set_early_response(int http_code, const s_Server &server);
    void set_early_redirect_response(int http_code, const std::string &location);

  public:
    RouterResource(const s_Server &server, const std::string &uri, const std::string &method);

    /**
     * Get the best matching config file location for the @resource
     *
     * Return: pointer for the config file location matching the resource or NULL
     */
    static const s_Location *get_best_matched_location(const s_Server &server, const std::string &resource);

    /**
     * Tells if @resource requested by the user matches the @location_path at the config file
     * so "/img" matches "/img" and "/img/x.png", but NOT "/images/x.png"
     *
     * Return: true meaning @loc_path is a possible match for @resource, or false meaning the opposite
     */
    static bool location_matcher(const std::string &resource, const std::string &loc_path);

    /**
     * Return: path of the default page for the @status_code or empty string meaning use your default, you don't have a
     * default? that you problem!
     *
     * NOTE: Each server could have muliple default paths foreach error page, this will get them for you
     */
    static std::string get_default_page(const s_Server &server, int status_code);

    /**
     * Returns: false if path  will traverse above the root directory, true if its not
     */
    static bool path_traverse_is_safe(const std::string &str);

    /**
     * Join two path segments
     *
     * The return value is the concatenation of two paths
     * I only use `/` so this screams NOT CROSS-PLATFORM, mybe something to fix later?
     */
    static std::string join_path(const std::string &root, const std::string &rest);

    /**
     * Return: true if the method is listed in the location's configuration, or if the method is "GET" (which
     * is implicitly allowed) or false if the method is restricted
     */
    static bool location_allowed_method(const s_Location &location, const std::string &method);

    const s_Location *get_location();
    RouterResult      get_early_router_result();
    std::string       get_disk_path();
    std::string       get_req_path();

    bool has_early_response();
    bool exists();
    bool is_directory();
    bool is_readable();
};

/**
 * Simple logging utility for debugging, sometime in the near future you'll ask what WTF that just happend this will
 * help!
 */
void router_log_helper(const std::string &resource, const s_Location *loc, const std::string &file_path,
                       const std::string &msg, int http_code);

#endif
