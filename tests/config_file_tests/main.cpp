#include "../../src/config_file_parser/parser/parser.hpp"
#include "../../src/config_file_parser/parser/configfile.hpp"
#include <iostream>
#include <iomanip>

/**
 * Helper to print Location details to verify inheritance and parsing
 */
void printLocation(const s_Location& loc, size_t index) {
    std::cout << "      [Location " << index << "] Path: " << (loc.path.empty() ? "/" : loc.path) << "\n";
    std::cout << "        |-- Root:         " << loc.root << "\n";
    std::cout << "        |-- Index:        " << loc.index << "\n";
    std::cout << "        |-- Max Body:     " << loc.max_body_size << " bytes\n";
    std::cout << "        |-- Autoindex:    " << (loc.autoindex ? "on" : "off") << "\n";
    
    if (!loc.methods.empty()) {
        std::cout << "        |-- Methods:      ";
        for (size_t i = 0; i < loc.methods.size(); ++i)
            std::cout << loc.methods[i] << (i == loc.methods.size() - 1 ? "" : ", ");
        std::cout << "\n";
    }

    if (loc.redirect_code != 0)
        std::cout << "        |-- Redirection:  " << loc.redirect_code << " -> " << loc.redirect_path << "\n";

    if (!loc.CGIhandlers.empty()) {
        std::cout << "        |-- CGI Handlers:\n";
        std::map<std::string, std::string>::const_iterator it;
        for (it = loc.CGIhandlers.begin(); it != loc.CGIhandlers.end(); ++it)
            std::cout << "            " << it->first << " -> " << it->second << "\n";
    }
    
    if (!loc.uploadStore.empty())
        std::cout << "        |-- Upload Store: " << loc.uploadStore << "\n";
    std::cout << "        --------------------------------------\n";
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./webserv <config_file>\n";
        return 1;
    }

    try
    {
        // 1. Lexical Analysis
        std::string file_name = argv[1];
        // lexer lx(file_name);
        // std::vector<s_token> tokens = lx.tokenize();

        // // 2. Parsing (Generating Directive Tree)
        // Parser parser(tokens); 
        // std::cout << "--- Directive Tree ---\n";
        // parser.print_directives();
        // std::cout << "----------------------\n\n";

        // // 3. Interpreting (Generating s_Server Vector)
        // Interpreter interpreter(parser.getDirectives()); 
        // std::vector<s_Server> servers = interpreter.getServers();
        ConfigFile config(file_name);
        std::vector<s_Server> servers = config.getConfig();
        // 4. Print Processed Configuration
        std::cout << "========== INTERPRETED CONFIGURATION ==========\n";
        for (size_t i = 0; i < servers.size(); ++i) {
            std::cout << "SERVER [" << i << "]\n";
            
            // Print Listeners (Map: IP -> Port)
            std::map<std::string, int>::const_iterator pit;
            for (pit = servers[i].ports.begin(); pit != servers[i].ports.end(); ++pit) {
                std::cout << "  - Listen: " << pit->first << ":" << pit->second << "\n";
            }

            std::cout << "  - Global Root:  " << servers[i].root << "\n";
            std::cout << "  - Global Index: " << servers[i].index << "\n";
            std::cout << "  - Max Body:     " << servers[i].max_body_size << " bytes\n";

            // Print Error Pages
            if (!servers[i].error_page.empty()) {
                std::cout << "  - Error Pages:  ";
                std::map<int, std::string>::const_iterator eit;
                for (eit = servers[i].error_page.begin(); eit != servers[i].error_page.end(); ++eit)
                    std::cout << "[" << eit->first << " -> " << eit->second << "] ";
                std::cout << "\n";
            }

            // Print Locations
            std::cout << "  - Locations:    (" << servers[i].locations.size() << ")\n";
            for (size_t j = 0; j < servers[i].locations.size(); ++j) {
                printLocation(servers[i].locations[j], j);
            }
            std::cout << "===============================================\n";
        }
        std::cout << "Configuration parsed and interpreted successfully.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}