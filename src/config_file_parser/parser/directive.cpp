#include "directive.hpp"

/**
 * Default constructor for the Directive class.
 */
Directive::Directive() : m_index(0)
{
}

/**
 * Constructor for the Directive class with a specified index.
 *
 * @param index The initial index for parsing tokens.
 */
Directive::Directive(int index) : m_index(index)
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
bool Directive::expect(t_type type, const s_token &token) const
{
    return (type == token.type);
}

/**
 * Increments the index for parsing tokens.
 */
void Directive::increment_index()
{
    m_index += 1;
}

//================ getters ================ //
/**
 * Gets the current index for parsing tokens.
 * @return The current index.
 */
int Directive::get_index() const
{
    return m_index;
}

/**
 * Gets the key of the directive.
 * @return The key of the directive.
 */
std::string &Directive::get_key()
{
    return m_key;
}

/**
 * Gets the values associated with the directive.
 * @return A reference to the vector of values.
 */
std::vector<std::string> &Directive::get_value()
{
    return m_values;
}

/**
 * Gets the children of the directive.
 * @return A reference to the vector of child directives.
 */
std::vector<Directive> &Directive::get_children()
{
    return m_children;
}

/**
 * Parses a directive from the given tokens starting at the specified index.
 * @param tokens The vector of tokens to parse.
 */
void Directive::parse_directive(const std::vector<s_token> &tokens)
{
    if (m_index >= (int)tokens.size())
        throw std::runtime_error("Unexpected EOF");
    if (tokens[m_index].type == WORD)
    {
        m_key = tokens[m_index].value;
        increment_index();
        while (m_index < (int)tokens.size() && (expect(WORD, tokens[m_index]) || expect(NUMBER, tokens[m_index])))
        {
            m_values.push_back(tokens[m_index].value);
            increment_index();
        }
        if (m_index < (int)tokens.size() && expect(SEMICOLON, tokens[m_index]))
        {
            increment_index();
            return;
        }
        else if (m_index < (int)tokens.size() && expect(LBRACE, tokens[m_index]))
        {
            increment_index();
            while (true)
            {
                if (m_index >= (int)tokens.size())
                    throw std::runtime_error("Unexpected EOF (missing '}')");

                if (tokens[m_index].type == RBRACE)
                    break;
                Directive node(m_index);
                node.parse_directive(tokens);
                m_children.push_back(node);
                if (node.get_index() == this->m_index)
                    throw std::runtime_error("Parser stuck");
                m_index = node.get_index();
            }
            increment_index();
            return;
        }
        else
        {
            if (m_index >= (int)tokens.size())
                throw std::runtime_error("Unexpected EOF");
            throw std::runtime_error("Unexpected token '" + tokens[m_index].value + "'");
        }
    }
    else
    {
        std::stringstream ss;
        ss << "Expected directive at line " << tokens[m_index].line;
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
