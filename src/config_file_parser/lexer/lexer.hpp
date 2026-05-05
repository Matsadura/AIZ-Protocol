#pragma once

#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

enum t_type
{
    LBRACE,
    RBRACE,
    SEMICOLON,
    WORD,
    NUMBER,
    UNKNOWN,
    EOF_TOKEN,
    TOKEN_NONE
};

struct s_token
{
    t_type type;
    std::string value;
    int line;

    s_token() : type(TOKEN_NONE), line(0)
    {
    }
};

class lexer
{
  private:
    std::string m_file_content;
    size_t m_pos;
    int m_line;

  public:
    lexer(const std::string &file_name);
    ~lexer();
    std::vector<s_token> tokens;
    std::vector<s_token> &tokenize();
    
  private:
    // helpers
    int collect_digits(std::string &str);
    void collect_words_and_numbers();
    bool isWordChar(char c);
    s_token createToken(const std::string &value, t_type type) const;
    void print_tokens(const std::vector<s_token> &tokens);
};