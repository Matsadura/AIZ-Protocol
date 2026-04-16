#include "lexer.hpp"

/**
 *
 */
lexer::lexer(std::string file_name) : pos(0), line(0)
{
    std::ifstream file(file_name);
    
    if(!file)
        throw std::runtime_error("Can not to open file");
    file_content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

lexer::~lexer(){}