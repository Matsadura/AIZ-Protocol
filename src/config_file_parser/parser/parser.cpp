#include "parser.hpp"
#include <cstddef>
#include <stdexcept>

/**
 * Default constructor for the parser class.
 */
parser::parser()
{
}

/**
 * Parameterized constructor for the parser class.
 *
 * @param key The key associated with the parser node.
 * @param values The values associated with the parser node.
 * @param children The child nodes of the parser node.
 */
parser::parser(const std::string &key, const std::vector<std::string> &values, const std::vector<parser> &children)
{
    m_key      = key;
    m_values   = values;
    m_children = children;
}

/**
 * Destructor for the parser class.
 */
parser::~parser()
{
}

bool    parser::expect(t_type type, const s_token &token)
{
    return (type == token.type);
}



void    parser::parse_directive(const std::vector<s_token> &tokens, int &index)
{
    if(index >= (int)tokens.size())
        throw std::runtime_error("Unexpected EOF");
    if(tokens[index].type == WORD)
    {
        m_key = tokens[index].value;
        index++;
        while(index < (int)tokens.size() && (expect(WORD, tokens[index]) ||
         expect(NUMBER, tokens[index])))
        {
            m_values.push_back(tokens[index].value);
            index++;
        }
        if(index < (int)tokens.size() && expect(SEMICOLON, tokens[index]))
        {
            index++;
            return;
        }
        else if(index < (int)tokens.size() && expect(LBRACE, tokens[index]))
        {
            index++;
            while(true)
            {
                if(index >= (int)tokens.size())
                    throw std::runtime_error("Unexpected EOF (missing '}')");

                if(tokens[index].type == RBRACE)
                    break;
                int old_index = index;
                parser node;
                node.parse_directive(tokens, index);
                m_children.push_back(node);
                if(index == old_index)
                    throw std::runtime_error("Parser stuck");
            }
            index++;
            return;
        }
        else {
            throw std::runtime_error("Unexpected token");
        }
    }
    else {
        std::stringstream ss;
        ss << "Expected directive at line " << tokens[index].line;
        throw std::runtime_error(ss.str());
    }   

}