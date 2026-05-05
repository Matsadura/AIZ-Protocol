#pragma once

#include "../lexer/lexer.hpp"

#include <vector>
#include <sstream>
class parser
{
  private:
    std::string m_key;
    std::vector<std::string> m_values;
    std::vector<parser> m_children;

  public:
    parser();
    parser(const std::string &key, const std::vector<std::string> &values, const std::vector<parser> &children);
    ~parser();
    void    parse_directive(const std::vector<s_token> &tokens, int &index);
    // void    parse(const std::vector<s_token> &tokens, int &index);
    bool    expect(t_type type, const s_token &token);


    void print(int depth) const
    {
        // indentation
        for (int i = 0; i < depth; i++)
            std::cout << "  ";

        // print key
        std::cout << m_key;

        // print values
        for (size_t i = 0; i < m_values.size(); i++)
            std::cout << " " << m_values[i];

        std::cout << std::endl;

        // print children recursively
        for (size_t i = 0; i < m_children.size(); i++)
            m_children[i].print(depth + 1);
    }

};