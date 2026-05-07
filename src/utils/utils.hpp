#ifndef UTILS_HPP
#define UTILS_HPP

#include <cctype>
#include <map>
#include <string>
#include <vector>

std::string trim(const std::string &str);
std::vector<std::string> split(const std::string &str, char delimiter);
bool isAllUpper(const std::string &str);
int hexToInt(char c);
std::string toLower(const std::string &str);
bool isNumeric(const std::string &str);

#endif /* UTILS_HPP */