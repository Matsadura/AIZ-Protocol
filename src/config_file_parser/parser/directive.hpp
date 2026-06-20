#pragma once

#include "../lexer/lexer.hpp"
#include <sstream>

class Directive
{
  private:
    std::string m_key;
    std::vector<std::string> m_values;
    std::vector<Directive> m_children;
    int m_index;

  public:
    Directive();
    Directive(int index);
    ~Directive();
    void parse_directive(const std::vector<s_token> &tokens);
    bool expect(t_type type, const s_token &token) const;
    void printDirective(int indent = 0) const;
    void increment_index();
    // getters
    int get_index() const;
    const std::string &get_key() const;
    const std::vector<std::string> &get_values() const;
    const std::vector<Directive> &get_children() const;
};