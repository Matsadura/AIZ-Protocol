#include "lexer.hpp"
#include <iostream>

/**
 * Constructor that reads the content of the config file
 * and stores it in a string
 * @file_name: the name of the config file
 * Throws: runtime_error if the file cannot be opened or is empty
 */
lexer::lexer(const std::string &file_name) : m_pos(0), m_line(1)
{
    std::ifstream file(file_name.c_str());

    if (!file)
        throw std::runtime_error("Cannot open file");
    m_file_content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    if (m_file_content.empty())
        throw std::runtime_error("Empty file");
}

/**
 * Destructor
 */
lexer::~lexer()
{
}

/**
 * Helper function that checks if a character is a valid WORD character
 * @c: The character to check
 */
bool lexer::isWordChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.' || c == '/' || c == ':' || c == '-';
}

/**
 * create tokens
 * @value: the value of the token
 * @type: type of the token
 * Returns: a token object
 */
s_token lexer::createToken(const std::string &value, t_type type) const
{
    s_token obj;
    obj.type  = type;
    obj.line  = m_line;
    obj.value = value;
    return obj;
}

/**
 * tokenize the config file
 * Returns: a vector of tokens
 */
std::vector<s_token> lexer::tokenize()
{
    tokens.clear();
    m_pos  = 0;
    m_line = 1;
    while (m_pos < m_file_content.size())
    {
        char current = m_file_content[m_pos];
        if (isspace(static_cast<unsigned char>(current)))
        {
            if (current == '\n')
                m_line++;
            m_pos++;
            continue;
        }
        else if (current == '{')
            tokens.push_back(createToken("{", LBRACE));
        else if (current == '}')
            tokens.push_back(createToken("}", RBRACE));
        else if (current == ';')
            tokens.push_back(createToken(";", SEMICOLON));
        else if (current == '#')
        {
            while (m_pos < m_file_content.size() && m_file_content[m_pos] != '\n')
                m_pos++;
            m_pos--;
        }
        else if (isWordChar(current))
        {
            collect_words_and_numbers();
            continue;
        }
        else
        {
            std::string value;
            tokens.push_back(createToken(value += current, UNKNOWN));
        }
        m_pos++;
    }
    return tokens;
}


/**
 * Helper function that collects a WORD or NUMBER from
 * the config file
 */
void lexer::collect_words_and_numbers()
{
    std::string value;
    int flag = 0;

    while (m_pos < m_file_content.size() && isWordChar(m_file_content[m_pos]))
    {
        value += m_file_content[m_pos];
        if (isdigit(static_cast<unsigned char>(m_file_content[m_pos])) == 0 && !flag)
            flag = 1;
        m_pos++;
    }
    if (flag == 0)
        tokens.push_back(createToken(value, NUMBER));
    else
        tokens.push_back(createToken(value, WORD));
}
