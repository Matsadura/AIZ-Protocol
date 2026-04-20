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
    UNKNOWN,
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
    private:
        // helpers
        int    collect_digits(std::string str);
        void    collect_words();
        void    collect_lbrace();
        void    collect_rbrace();
        void    collect_semicolon();
        bool    isWordChar(char c);
        struct token    createToken(std::string value, type type);

};