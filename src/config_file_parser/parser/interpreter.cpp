#include "directive.hpp"
# include "interperter.hpp"
#include <cstddef>
#include <stdexcept>
#include <vector>




/**
 * Constructor for the Interpreter class.
 * @param directives A vector of Directive objects representing the parsed configuration directives.
 */
Interperter::Interperter(const std::vector<Directive> &directives)
{
    std::vector<Server> srvs;

    for(size_t i = 0; i < directives.size(); i++)
    {
        if(directives[i].get_key() == "server")
        {
            srvs.push_back(parseServer(directives[i]));
        }
        else 
        {
            throw std::runtime_error("Unknown top-level directive: " + directives[i].get_key());
        }
    }
}

/**
 * server parser
 * @param directive The Directive object representing the server block to parse.
 * @return A Server object populated with the parsed configuration from the directive.
 */
Server Interperter::parseServer(const Directive &directive)
{
    Server srv;
    std::vector<Directive> DirectiveChildren;
    
    DirectiveChildren = directive.get_children();
    for(size_t i = 0; i < DirectiveChildren.size(); i++)
    {
        if(DirectiveChildren[i].get_key() == "listen")
            // srv.ports = handelport
        else if(DirectiveChildren[i].get_key() == "max_body_size")
            // srv.max_body_size = handlemaxbodysize
        else if(DirectiveChildren[i].get_key() == "root")
            //srv.root = handleroot
        else if(DirectiveChildren[i].get_key() == "location")
        {
            //srv.location = handlelocation(DirectiveChildren, index, &srv.location)
        }
    }
    return srv;
}

