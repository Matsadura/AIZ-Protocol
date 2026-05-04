#include "utils.hpp"
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

/**
 * isAllUpper - Checks if a string consists entirely of uppercase letters.
 * @str: The input string to be checked.
 * Return: true if the string is all uppercase, false otherwise.
 */
bool isAllUpper(const std::string &str)
{
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (!std::isupper(static_cast<unsigned char>(str[i])))
            return false;
    }
    return true;
}

/**
 * hexToInt - Converts a hexadecimal character to its integer value.
 * @c: The hexadecimal character to be converted (0-9, a-f, A-F).
 * Return: The integer value of the hexadecimal character, or -1 if invalid.
 */
int hexToInt(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

/**
 * toLower - Converts a string to lowercase.
 * @str: The input string to be converted.
 * Return: A new string with all characters converted to lowercase.
 */
std::string toLower(const std::string &str)
{
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i)
    {
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    }
    return result;
}

/**
 * isDuplicateHeader - Checks if a header key already exists in the headers map.
 * @headers: The map of existing headers.
 * @key: The header key to check for duplication.
 * Return: true if the header key already exists, false otherwise.
 */
bool isDuplicateHeader(const std::map<std::string, std::string> &headers, const std::string &key)
{
    return headers.find(key) != headers.end();
}

/**
 * isAlphaNumeric - Checks if a string consists entirely of alphanumeric characters.
 * @str: The input string to be checked.
 * Return: true if the string is alphanumeric, false otherwise.
 */
bool isAlphaNumeric(const std::string &str)
{
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (!std::isalnum(static_cast<unsigned char>(str[i])))
            return false;
    }
    return true;
}
