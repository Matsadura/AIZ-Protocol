#include "utils.hpp"
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
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
    std::string              token;
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
 * isNumeric - Checks if a string consists entirely of alphanumeric characters.
 * @str: The input string to be checked.
 * Return: true if the string is numeric, false otherwise.
 */
bool isNumeric(const std::string &str)
{
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(str[i])))
            return false;
    }
    return true;
}

bool string_starts_with(const std::string &bigger_string, const std::string &prefix)
{
    if (bigger_string.length() < prefix.length())
    {
        return false;
    }
    return std::equal(prefix.begin(), prefix.end(), bigger_string.begin());
}

bool is_file_or_directory_exists(const std::string &path)
{
    struct stat file_info = {};

    return stat(path.c_str(), &file_info) == 0;
}

bool is_file_regular(const std::string &path)
{
    struct stat file_info = {};

    return stat(path.c_str(), &file_info) == 0 && S_ISREG(file_info.st_mode);
}

bool is_directory_path(const std::string &path)
{
    struct stat file_info = {};

    return stat(path.c_str(), &file_info) == 0 && S_ISDIR(file_info.st_mode);
}

bool is_file_readable(const std::string &filepath)
{
    return access(filepath.c_str(), R_OK) == 0;
}

bool is_file_executable(const std::string &filepath)
{
    return access(filepath.c_str(), X_OK) == 0;
}

bool isTokenChar(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '!' || c == '#' ||
           c == '$' || c == '%' || c == '&' || c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' || c == '^' ||
           c == '_' || c == '`' || c == '|' || c == '~';
}

bool isValidHeaderName(const std::string &key)
{
    if (key.empty())
        return false;
    for (size_t i = 0; i < key.size(); ++i)
    {
        if (!isTokenChar(key[i]))
            return false;
    }
    return true;
}

bool isValidHeaderValue(const std::string &value)
{
    for (size_t i = 0; i < value.size(); ++i)
    {
        if ((value[i] <= 31 && value[i] != '\t') || value[i] == 127)
            return false;
    }
    if (value.size() > 8192)
        return false;
    return true;
}
