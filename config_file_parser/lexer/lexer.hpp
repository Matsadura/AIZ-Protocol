#pragma once

#include "iostream"
#include "vector"
#include "fstream"
#include "iterator"
enum type
{
    LBRACE,
    RBRACE,
    SEMICOLON,
    WORD,
    NUMBER,
    EOF_TOKEN
};

struct token
{
    type type;
    std::string value;
    int line;
};

class lexer
{
    private:
        std::string file_content;
        size_t pos;
        int line;
    public:
        lexer(std::string file_name);
        ~lexer();
        std::vector<token> tokens;
        std::vector<token>  tokenize();
};