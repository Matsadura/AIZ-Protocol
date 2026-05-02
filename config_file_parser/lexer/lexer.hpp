#pragma once

#include "iostream"
#include "vector"
#include "fstream"
#include "iterator"
enum t_type
{
    LBRACE,
    RBRACE,
    SEMICOLON,
    WORD,
    NUMBER,
    UNKNOWN,
    EOF_TOKEN
};

struct t_token
{
    t_type type;
    std::string value;
    int line;
};

class lexer
{
    private:
        std::string m_file_content;
        size_t m_pos;
        int m_line;
    public:
        lexer(std::string &file_name);
        ~lexer();
        std::vector<t_token> tokens;
        std::vector<t_token>  tokenize();
    private:
        // helpers
        int    collect_digits(std::string str);
        void    collect_words();
        bool    isWordChar(char c);
        struct t_token    createToken(const std::string &value, t_type type) const;

};