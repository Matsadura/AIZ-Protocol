#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <cctype>
#include <vector>

std::string trim(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);

#endif /* UTILS_HPP */