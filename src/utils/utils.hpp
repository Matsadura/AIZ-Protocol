#ifndef UTILS_HPP
#define UTILS_HPP

#include <cctype>
#include <string>
#include <vector>

std::string trim(const std::string &str);
std::vector<std::string> split(const std::string &str, char delimiter);
bool isAllUpper(const std::string &str);

#endif /* UTILS_HPP */