#include "lexer.hpp"
<<<<<<< HEAD
#include "../parser/parser.hpp"
#include <vector>
=======
>>>>>>> origin/main

void print_tokens(const std::vector<s_token> &tokens)
{
    for (size_t i = 0; i < tokens.size(); i++)
    {
        std::cout << "line " << tokens[i].line << " | ";

        if (tokens[i].type == LBRACE)
            std::cout << "LBRACE";
        else if (tokens[i].type == RBRACE)
            std::cout << "RBRACE";
        else if (tokens[i].type == SEMICOLON)
            std::cout << "SEMICOLON";
        else if (tokens[i].type == NUMBER)
            std::cout << "NUMBER";
        else if (tokens[i].type == WORD)
            std::cout << "WORD";
        else if (tokens[i].type == UNKNOWN)
            std::cout << "UNKNOWN";

        std::cout << " | value: [" << tokens[i].value << "]\n";
    }
}

std::vector<parser> parse(const std::vector<s_token> &tokens, int &index)
{
    std::vector<parser> vector;
    while(index < (int)tokens.size())
    {
        parser obj;
        obj.parse_directive(tokens, index);
        vector.push_back(obj);

    }
    return vector;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./lexer <config_file>\n";
        return 1;
    }

    try
    {
        std::string file_name = argv[1];
        lexer lx(file_name);
        std::vector<s_token> tokens = lx.tokenize();
        int i = 0;
        std::vector<parser> vector = parse(tokens, i);
        
        
        // print_tokens(tokens);
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}