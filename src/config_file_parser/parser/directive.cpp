#include "directive.hpp"

/**
 * Default constructor for the Directive class.
 */
Directive::Directive()
{
}

/**
 * Destructor for the Directive class.
 */
Directive::~Directive()
{
}

/**
 * Helper function to check if the token type matches the expected type.
 *
 * @param type The expected token type.
 * @param token The token to check.
 * @return True if the token type matches the expected type, false otherwise.
 */
bool Directive::expect(t_type type, const s_token &token)
{
    return (type == token.type);
}

/**
 * Parses a directive from the given tokens starting at the specified index.
 *
 * @param tokens The vector of tokens to parse.
 * @param index The current index in the tokens vector (passed by reference).
 * @throws std::runtime_error If an unexpected token is encountered or if EOF is reached unexpectedly.
 */
void Directive::parse_directive(const std::vector<s_token> &tokens, int &index)
{
    if (index >= (int)tokens.size())
        throw std::runtime_error("Unexpected EOF");
    if (tokens[index].type == WORD)
    {
        m_key = tokens[index].value;
        index++;
        while (index < (int)tokens.size() && (expect(WORD, tokens[index]) || expect(NUMBER, tokens[index])))
        {
            m_values.push_back(tokens[index].value);
            index++;
        }
        if (index < (int)tokens.size() && expect(SEMICOLON, tokens[index]))
        {
            index++;
            return;
        }
        else if (index < (int)tokens.size() && expect(LBRACE, tokens[index]))
        {
            index++;
            while (true)
            {
                if (index >= (int)tokens.size())
                    throw std::runtime_error("Unexpected EOF (missing '}')");

                if (tokens[index].type == RBRACE)
                    break;
                int old_index = index;
                Directive node;
                node.parse_directive(tokens, index);
                m_children.push_back(node);
                if (index == old_index)
                    throw std::runtime_error("Parser stuck");
            }
            index++;
            return;
        }
        else
        {
            if (index >= (int)tokens.size())
                throw std::runtime_error("Unexpected EOF");
            throw std::runtime_error("Unexpected token '" + tokens[index].value + "'");
        }
    }
    else
    {
        std::stringstream ss;
        ss << "Expected directive at line " << tokens[index].line;
        throw std::runtime_error(ss.str());
    }
}

/**
 * Prints the directive and its children with indentation for better readability.
 *
 * @param indent The number of spaces to indent the output (default is 0).
 */
void Directive::printDirective(int indent) const
{
    std::string spaces(indent, ' ');

    std::cout << spaces << this->m_key;

    for (size_t i = 0; i < this->m_values.size(); i++)
        std::cout << " " << this->m_values[i];

    std::cout << std::endl;

    for (size_t i = 0; i < this->m_children.size(); i++)
        this->m_children[i].printDirective(indent + 4);
}
