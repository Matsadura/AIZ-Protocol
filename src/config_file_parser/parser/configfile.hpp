#pragma once

#include "interpreter.hpp"
#include <string>

class ConfigFile
{
  private:
    std::vector<s_Server> m_ConfigData;

  public:
    ConfigFile(const std::string &file_name);
    ~ConfigFile();

    std::vector<s_Server> getConfig() const;
    s_Server getServerById(int ID) const;
};