#pragma once

#include "directive.hpp"
#include "parser.hpp"
#include <map>
#include <vector>

struct Location
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
    std::string redirect_code;
    std::string redirect_path;
};

struct Server
{
    std::vector<std::pair<std::string, int> > ports;
    std::string max_body_size;
    
    std::string global_root;
    std::string global_index;
    // Locations
    std::vector<Location> locations;
};

class Interperter
{
    private:
        std::vector<Server> m_servers;
    public:
        Interperter(const std::vector<Directive> &directives);
        ~Interperter();
    private:
        Server parseServer(const Directive &directive);
};