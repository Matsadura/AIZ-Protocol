#include "../includes/utils.hpp"
#include <iostream>
#include <vector>

/**
 * split - Splits a string into a vector of strings based on a specified delimiter.
 * @str: The input string to be split.
 * @delimiter: The character used to split the string.
 * Return: A vector of strings resulting from the split operation.
 */
std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
    {
        char ch = *it;
        if (ch == delimiter)
        {
            if (!token.empty())
            {
                tokens.push_back(token);
                token.clear();
            }
        }
        else
        {
            token += ch;
        }
    }
    if (!token.empty())
        tokens.push_back(token);
    return tokens;
}

/**
 * trim - Removes leading and trailing whitespace from a string.
 * @str: The input string to be trimmed.
 * Return: A new string with leading and trailing whitespace removed.
 */
std::string trim(const std::string &str)
{
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last - first + 1);
}