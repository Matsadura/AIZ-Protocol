#include "parser.hpp"
#include "directive.hpp"

/**
 * Constructor for the Parser class.
 * @tokens: A vector of tokens to be parsed.
 */
Parser::Parser(const std::vector<s_token> &tokens)
{
    int index = 0;
    while (index < (int)tokens.size())
    {
        Directive obj;
        obj.parse_directive(tokens, index);
        m_directives.push_back(obj);
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
