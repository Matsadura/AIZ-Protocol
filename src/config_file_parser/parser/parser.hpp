#pragma once

#include "../lexer/lexer.hpp"

#include <sstream>
#include <vector>

#include "directive.hpp"

class Parser
{
  private:
    std::vector<Directive> m_directives;

  public:
    Parser(const std::vector<s_token> &tokens);
    ~Parser();
    void print_directives();
    std::vector<Directive> getDirectives() const;
};