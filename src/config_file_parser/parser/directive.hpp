#pragma once

#include "../lexer/lexer.hpp"
#include <sstream>

class Directive
{
  private:
    std::string m_key;
    std::vector<std::string> m_values;
    std::vector<Directive> m_children;

  public:
    Directive();
    ~Directive();
    void parse_directive(const std::vector<s_token> &tokens, int &index);
    bool expect(t_type type, const s_token &token);
    void printDirective(int indent = 0) const;
};