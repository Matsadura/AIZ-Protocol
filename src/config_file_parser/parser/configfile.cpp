#include "configfile.hpp"
#include "interpreter.hpp"

/**
 * Constructor: Parses the configuration file and initializes the server configurations
 * @param file_name Path to the configuration file to be parsed
 */
ConfigFile::ConfigFile(const std::string &file_name)
{
    lexer lx(file_name);
    std::vector<s_token> tokens = lx.tokenize();

    Parser parser(tokens);

    Interpreter interpreter(parser.getDirectives());
    m_ConfigData = interpreter.getServers();
}

/**
 * Destructor
 */
ConfigFile::~ConfigFile()
{
}

/**
 * Getter for the parsed configuration data
 * @return Vector of s_Server structures representing the configuration
 */
std::vector<s_Server> ConfigFile::getConfig() const
{
    return m_ConfigData;
}

/**
 * Getter for a specific server configuration by ID
 * @param ID Index of the server configuration to retrieve
 * @return s_Server structure representing the specified server configuration
 * @throws std::out_of_range if the ID is invalid
 */
s_Server ConfigFile::getServerConfig(int ID) const
{
    if (ID < 0 || static_cast<size_t>(ID) >= m_ConfigData.size())
        throw std::out_of_range("Server ID out of range");
    return m_ConfigData[ID];
}

std::string ConfigFile::getServerPath(int server_ID, const std::string &uri) const
{
    s_Server server = getServerConfig(server_ID);
    for (size_t i = 0; i < server.locations.size(); ++i)
    {
        if (server.locations[i].path == uri)
        {
            //parse_root
        }
    }
    return server.root + uri; // Default to server root if no location matches
}

