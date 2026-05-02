#include "lexer.hpp"

/**
 * constructor
 * @file_name: The configuration file name
 */
lexer::lexer(std::string &file_name) : m_pos(0), m_line(0)
{
    std::ifstream file(file_name.c_str());
    
    if(!file)
        throw std::runtime_error("Cannot open file");
    m_file_content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    if(m_file_content.empty())
        throw std::runtime_error("Empty file");
}

/**
 * Destructor
 */
lexer::~lexer(){}

/**
 * Helper function that detects valid characters
 * @c = the character
 */
bool    lexer::isWordChar(char c)
{
    return std::isalnum(c) || c == '_' || c == '.' || c == '/' || c == ':' || c == '-';
}

/**
 * create tokens
 * @value = the value of the token
 * @type = type of the token
 */
struct t_token    lexer::createToken(const std::string &value, t_type type) const
{
    struct t_token obj;
    obj.type = type;
    obj.line = m_line;
    obj.value = value;
    return obj;
}

/**
 * tokenizer
 */
std::vector<t_token>  lexer::tokenize()
{
    while (m_pos < m_file_content.size())
    {
        char current = m_file_content[m_pos];
        if (isspace(current)) {
            if (current == '\n') m_line++;
            m_pos++;
            continue;
        }
        else if(current == '{')
            tokens.push_back(createToken("{", LBRACE));
        else if(current == '}')
            tokens.push_back(createToken("}", RBRACE));
        else if(current == ';')
            tokens.push_back(createToken(";", SEMICOLON));
        else if(current == '#')
        {
            while (m_pos < m_file_content.size() && m_file_content[m_pos] != '\n')
                m_pos++;
            m_pos--;
        }
        else if (isWordChar(current))
        {
            collect_words();
            continue;
        }
        else
        {
            std::string value;
            tokens.push_back(createToken( value+=current, UNKNOWN));
        }
        m_pos++;
    }
    return tokens;
}

/**
 * Helper function that collects a NUMBER from
 * the config  file
 */
int    lexer::collect_digits(std::string str)
{
    struct t_token obj;

    for(int i = 0; str[i]; i++)
    {
        if(!isdigit(static_cast<unsigned char>(str[i])))
            return 0;
    }
    tokens.push_back(createToken(str, NUMBER));
    return 1;
}

/**
 * Helper function that collects a WORD from
 * the config file and check if it's a number
 */
void    lexer::collect_words()
{
    std::string value;
    struct t_token obj;

    while(m_pos < m_file_content.size() && isWordChar(m_file_content[m_pos]) )
    {
        value += m_file_content[m_pos];
        m_pos++;
    }
    if(collect_digits(value))
        return;
    tokens.push_back(createToken(value, WORD));
}
