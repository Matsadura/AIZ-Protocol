#include "directive.hpp"
#include "interperter.hpp"
#include <algorithm>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * Constructor for the Interpreter class.
 * @param directives A vector of Directive objects representing the parsed configuration directives.
 */
Interperter::Interperter(const std::vector<Directive> &directives)
{
    std::vector<s_Server> srvs;

    for (size_t i = 0; i < directives.size(); i++)
    {
        if (directives[i].get_key() == "server")
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
s_Server Interperter::parseServer(const Directive &directive)
{
    s_Server srv;
    std::vector<Directive> DirectiveChildren;

    DirectiveChildren = directive.get_children();
    for (size_t i = 0; i < DirectiveChildren.size(); i++)
    {
        if (DirectiveChildren[i].get_key() == "listen")
        {
            if (DirectiveChildren[i].get_values().empty() || DirectiveChildren[i].get_values().size() > 1)
                throw std::runtime_error("Listen directive must have exactly one value");
            handleport(DirectiveChildren[i].get_values()[0], srv.ports);
        }
        else if (DirectiveChildren[i].get_key() == "max_body_size")
            srv.max_body_size = handlemaxbodysize(DirectiveChildren[i].get_values());
        else if (DirectiveChildren[i].get_key() == "root")
        {
            if (DirectiveChildren[i].get_values().empty() || DirectiveChildren[i].get_values().size() > 1)
                throw std::runtime_error("Root directive must have exactly one value");
            srv.global_root = DirectiveChildren[i].get_values()[0];
        }
        else if (DirectiveChildren[i].get_key() == "index")
        {
            if (DirectiveChildren[i].get_values().empty() || DirectiveChildren[i].get_values().size() > 1)
                throw std::runtime_error("Index directive must have exactly one value");
            srv.global_index = DirectiveChildren[i].get_values()[0];
        }
        else if (DirectiveChildren[i].get_key() == "error_page")
            handleErrorPage(DirectiveChildren[i].get_values(), srv.error_page);
        else if (DirectiveChildren[i].get_key() == "location")
        {
            srv.locations.push_back(handleLocation(DirectiveChildren, i, srv));
        }
        else
            throw std::runtime_error("Unknown directive in server block: " + DirectiveChildren[i].get_key());
    }
    return srv;
}

void Interperter::handleErrorPage(const std::vector<std::string> &values,
                                  std::map<int, std::string> &error_page)
{
    if(values.size() != 2)
        throw std::runtime_error("Error page directive must have exactly two values: error code and page path");
    char *rest;
    double error_code = strtod(values[0].c_str(), &rest);
    if(*rest)
        throw std::runtime_error("Error code must be a number");
    if(error_code < 400 || error_code > 599)
        throw std::runtime_error("Invalid error code: " + values[0]);
    error_page[static_cast<int>(error_code)] = values[1];
}

/**
 * Port parser
 * @param value A string representing the value of the listen directive, which can be in the format "IP:port" or just
 * "port".
 * @param ports A reference to a map where the parsed IP and port will be stored. The key is the IP address and the
 * value is the port number.
 * @return A reference to the updated map containing the parsed IP and port information.
 */
void Interperter::handleport(const std::string &value, std::map<std::string, int> &ports)
{
    size_t pos = value.find(':');

    if (pos != std::string::npos)
    {
        std::string ip       = value.substr(0, pos);
        std::string port_str = value.substr(pos + 1);
        char *rest;
        double num = strtod(port_str.c_str(), &rest);
        if (*rest != '\0')
            throw std::runtime_error("Port must be a number");
        if (num < 1 || num > 65535)
            throw std::runtime_error("Invalid port number: " + port_str);
        ports[ip] = static_cast<int>(num);
    }
    else
    {
        if (value.find('.') != std::string::npos)
        {
            ports[value] = 80; // default port for IP addresses
        }
        else
        {
            char *rest;
            double num = strtod(value.c_str(), &rest);
            if (*rest != '\0')
                throw std::runtime_error("Port must be a number");
            if (num < 1 || num > 65535)
                throw std::runtime_error("Invalid port number: " + value);
            ports["0.0.0.0"] = static_cast<int>(num); // default IP for port-only listen directives
        }
    }
}

/**
 * Max body size parser
 * @param values A vector of strings representing the values of the max_body_size directive, which should contain
 * exactly one value with an optional unit (e.g., "10M", "500K").
 * @return A size_t value representing the parsed maximum body size in bytes.
 */
size_t Interperter::handlemaxbodysize(const std::vector<std::string> &values)
{
    if (values.empty() || values.size() > 1)
        throw std::runtime_error("max_body_size directive must have exactly one value");
    char *rest;
    double num = std::strtod(values[0].c_str(), &rest);
    if (!*rest)
        return (static_cast<size_t>(num));
    else
    {
        std::string unit = rest;
        if (unit == "k" || unit == "K")
            return (static_cast<size_t>(num * 1024));
        else if (unit == "m" || unit == "M")
            return (static_cast<size_t>(num * 1024 * 1024));
        else if (unit == "g" || unit == "G")
            return (static_cast<size_t>(num * 1024 * 1024 * 1024));
        else
            throw std::runtime_error("Invalid unit for max_body_size: " + unit);
    }
    return 0; // This line will never be reached, but it's here to satisfy the compiler's requirement for a return
              // value.
}

/**
 * location parser
 * @param DirectiveChildren A vector of Directive objects representing the child directives within a location block.
 * @param i A reference to the current index in the DirectiveChildren vector, which will be updated as directives are
 * processed.
 * @param location A reference to an s_Location object that will be populated with the parsed configuration from the
 * location directives.
 * @return A reference to the populated s_Location object after processing the location directives.
 */
s_Location &Interperter::handleLocation(const std::vector<Directive> &DirectiveChildren, size_t &i, s_Server &server)
{
    s_Location location;

    location.root  = server.global_root;
    location.index = server.global_index;
    location.path  = DirectiveChildren[i].get_values()[0];
    for (; i < DirectiveChildren.size(); i++)
    {
        if (DirectiveChildren[i].get_key() == "methods")
            location.methods = handleMethods(DirectiveChildren[i].get_values());
        else if (DirectiveChildren[i].get_key() == "root" && location.root.empty())
        {
            if (DirectiveChildren[i].get_values().empty() || DirectiveChildren[i].get_values().size() > 1)
                throw std::runtime_error("Root directive must have exactly one value");
            location.root = DirectiveChildren[i].get_values()[0];
        }
        else if (DirectiveChildren[i].get_key() == "index" && location.index.empty())
        {
            if (DirectiveChildren[i].get_values().empty() || DirectiveChildren[i].get_values().size() > 1)
                throw std::runtime_error("Index directive must have exactly one value");
            location.index = DirectiveChildren[i].get_values()[0];
        }
        else if (DirectiveChildren[i].get_key() == "autoindex")
        {
            location.autoindex = handleAutoindex(DirectiveChildren[i].get_values()[0]);
        }
        else if (DirectiveChildren[i].get_key() == "upload_store")
        {
            if (DirectiveChildren[i].get_values().empty() || DirectiveChildren[i].get_values().size() > 1)
                throw std::runtime_error("Upload store directive must have exactly one value");
            location.uploadStore = DirectiveChildren[i].get_values()[0];
        }
        else if (DirectiveChildren[i].get_key() == "return")
        {
            if (DirectiveChildren[i].get_values().size() != 2)
                throw std::runtime_error("Return directive must have exactly two values");
            location.redirect_code = handleRedirectCode(DirectiveChildren[i].get_values()[0]);
            location.redirect_path = DirectiveChildren[i].get_values()[1];
        }
        else if (DirectiveChildren[i].get_key() == "cgi_ext")
        {
            location.CGIhandlers = handleCGI(DirectiveChildren[i].get_values());
        }
        else
            throw std::runtime_error("Unknown directive in location block: " + DirectiveChildren[i].get_key());
    }
}

/**
 * Methods parser
 * @param Methods A vector of strings representing the allowed HTTP methods.
 * @return A vector of strings containing the validated HTTP methods.
 */
std::vector<std::string> Interperter::handleMethods(const std::vector<std::string> &Methods)
{
    if (Methods.empty())
        throw std::runtime_error("HTTP Method field is empty");
    for (size_t i = 0; i < Methods.size(); i++)
    {
        if (Methods[i] != "GET" && Methods[i] != "POST" && Methods[i] != "DELETE")
        {
            throw std::runtime_error("HTTP Method not supported");
        }
    }
    return Methods;
}

/**
 * Autoindex parser
 * @param value A string representing the value of the autoindex directive, expected to be either "on" or "off".
 * @return A boolean value where true represents "on" and false represents "off". If the value is invalid, an exception
 * is thrown.
 */
bool Interperter::handleAutoindex(const std::string &value)
{
    if (value == "on")
        return true;
    else if (value == "off")
        return false;
    else
        throw std::runtime_error("Autoindex directive must be either 'on' or 'off'");
}

/**
 * Redirect code parser
 * @param code A string representing the redirect code.
 * @return An integer representing the validated redirect code.
 */
int Interperter::handleRedirectCode(const std::string &code)
{
    if (code.empty())
        throw std::runtime_error("Redirect code is empty");

    char *rest;
    double num = strtod(code.c_str(), &rest);
    if (*rest != '\0')
        throw std::runtime_error("Redirect code must be a number");
    if (num < 300 || num > 399)
        throw std::runtime_error("Invalid redirect code: " + code);
    return static_cast<int>(num);
}

/**
 * CGI parser
 * @param cgi_values A vector of strings representing the CGI directive values.
 * @return A map containing the CGI extensions and their corresponding handler paths.
 */
std::map<std::string, std::string> Interperter::handleCGI(const std::vector<std::string> &cgi_values)
{
    if (cgi_values.size() != 2)
        throw std::runtime_error("CGI directive must have exactly two values: extension and handler path");
    std::map<std::string, std::string> cgi_handler;
    cgi_handler[cgi_values[0]] = cgi_values[1];
    return cgi_handler;
}
