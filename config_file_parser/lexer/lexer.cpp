#include "lexer.hpp"

/**
 * constructor
 * @file_name: The configuration file name
 */
lexer::lexer(std::string file_name) : pos(0), line(0)
{
    std::ifstream file(file_name);
    
    if(!file)
        throw std::runtime_error("Can not to open file");
    file_content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    if(file_content.empty())
        throw std::runtime_error("Empty file");
}

/**
 * Destructor
 */
lexer::~lexer(){}

/**
 * tokenizer
 */
std::vector<token>  lexer::tokenize()
{
    std::string str = file_content;
    for(; str[pos]; pos++)
    {
        if(str[pos] == ' ' || str[pos] == '\t')
            continue;
        if(str[pos] == '\n')
            line++;
        if(isdigit(str[pos]))
            collect_digits();
        if(isalpha(str[pos]))
            collect_words();

    }
}

/**
 * Helper function that collects a NUMBER from
 * the config  file
 */
// void    lexer::collect_digits()
// {
//     std::string str = file_content;
//     std::string value;
//     struct token obj;

//     for(; str[pos]; pos++)
//     {
//         if(!isdigit(str[pos]))
//             break;
//         value += str[pos];
//     }
//     obj.value = value;
//     obj.type = NUMBER;
//     obj.line = line;
//     tokens.push_back(obj);
// }

/**
 * Helper function that collects a WORD from
 * the config file
 */
void    lexer::collect_words()
{
    std::string str = file_content;
    std::string value;
    struct token obj;

    for(; str[pos]; pos++)
    {
        if(!isalpha(str[pos]) && str[pos] != '_')
            break;
        value += str[pos];
    }
    obj.value = value;
    obj.type = WORD;
    obj.line = line;
    tokens.push_back(obj);
}

// I should modify collect_words to make it handle everything that is not a symbol and then check it is a number