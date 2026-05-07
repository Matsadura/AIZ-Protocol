#include "parser.hpp"
#include "directive.hpp"

/**
 * Constructor for the Parser class.
 * @tokens: A vector of tokens to be parsed.
 */
Parser::Parser(const std::vector<s_token> &tokens)
{
    if (tokens.empty())
        throw std::runtime_error("No tokens to parse");
    int index = 0;
    while (index < (int)tokens.size())
    {
        Directive obj(index);
        obj.parse_directive(tokens);
        m_directives.push_back(obj);
        index = obj.get_index();
    }
}

/**
 * Destructor for the Parser class.
 */
Parser::~Parser()
{
}

/**
 * Prints all the directives parsed by the parser.
 */
void Parser::print_directives()
{
    for (size_t i = 0; i < m_directives.size(); i++)
    {
        m_directives[i].printDirective();
    }
}
