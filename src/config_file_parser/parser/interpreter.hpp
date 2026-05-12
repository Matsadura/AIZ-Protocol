#pragma once

#include "directive.hpp"
#include "parser.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include <cstdlib>

struct s_Location
{
    std::string path;
    std::string root;
    std::string index;

    // methods
    std::vector<std::string> methods;

    // upload store
    std::string uploadStore;

    // cgi
    std::map<std::string, std::string> CGIhandlers;

    // redirect
    int redirect_code;
    std::string redirect_path;
    size_t max_body_size;

    bool autoindex;
};

struct s_Server
{
    std::map<std::string, int> ports;
    size_t max_body_size;

    std::string global_root;
    std::string global_index;
    std::string server_name;
    // Locations
    std::vector<s_Location> locations;
    std::map<int, std::string> error_page;
};

class Interpreter
{
  private:
    std::vector<s_Server> m_servers;

  public:
    Interpreter(const std::vector<Directive> &directives);
    ~Interpreter();

    std::vector<s_Server> getServers() const;   
  private:
    s_Server parseServer(const Directive &directive);
    s_Location handleLocation(const std::vector<Directive> &DirectiveChildren,
                                        s_Server &server, std::string path);
    void handleport(const std::string &value, std::map<std::string, int> &ports);
    std::vector<std::string>    handleMethods(const std::vector<std::string> &Methods);
    bool handleAutoindex(const std::string &value);
    int handleRedirectCode(const std::string &code);
    std::map<std::string, std::string> handleCGI(const std::vector<std::string> &cgi_values);
    size_t handlemaxbodysize(const std::vector<std::string> &values);
    void handleErrorPage(const std::vector<std::string> &values,
                                  std::map<int, std::string> &error_page);

};