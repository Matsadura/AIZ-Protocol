#ifndef UTILS_HPP
#define UTILS_HPP

#include <cctype>
#include <map>
#include <string>
#include <vector>

std::string              trim(const std::string &str);
std::vector<std::string> split(const std::string &str, char delimiter);
bool                     isAllUpper(const std::string &str);
int                      hexToInt(char c);
std::string              toLower(const std::string &str);
bool                     isNumeric(const std::string &str);

/**
 * Return: true if @path is a regular file
 */
bool is_file_regular(const std::string &path);

/**
 * Return: true if @filepath is can be read
 */
bool is_file_readable(const std::string &filepath);

/**
 * Return: true if @filepath is can be executed
 */
bool is_file_executable(const std::string &filepath);

/**
 * Returns a Boolean stating whether a string starts with the specified prefix
 */
bool string_starts_with(const std::string &bigger_string, const std::string &prefix);

bool isTokenChar(char c);
bool isValidHeaderValue(const std::string &value);
bool isValidHeaderName(const std::string &key);

#endif /* UTILS_HPP */
