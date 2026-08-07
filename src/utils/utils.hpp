#ifndef UTILS_HPP
#define UTILS_HPP

#include <cctype>
#include <fstream>
#include <iostream>
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
 * Return: true if path if a directory
 */
bool is_directory_path(const std::string &path);

/**
 * Return: true if @filepath is can be read
 */
bool is_file_readable(const std::string &filepath);

/**
 * Return: true if @filepath is can be executed
 */
bool is_file_executable(const std::string &filepath);

/**
 * Return: true if @filepath exists
 */
bool is_file_or_directory_exists(const std::string &path);

/**
 * Returns a Boolean stating whether a string starts with the specified prefix
 */
bool string_starts_with(const std::string &bigger_string, const std::string &prefix);

bool isTokenChar(char c);
bool isValidHeaderValue(const std::string &value);
bool isValidHeaderName(const std::string &key);

/**
 * Convert integer to std::string
 */
std::string int_to_string(int n);

class DebugStore
{
  public:
    struct RawExchange
    {
        std::string request;
        std::string response;
    };

  private:
    bool                       m_enabled;
    std::map<int, RawExchange> m_data;

    DebugStore() : m_enabled(false)
    {
    }

  public:
    static DebugStore &instance()
    {
        static DebugStore inst;
        return inst;
    }

    void enable(bool v)
    {
        m_enabled = v;
    }

    bool enabled() const
    {
        return m_enabled;
    }

    void append_request(int fd, const char *data, long len)
    {
        if (!m_enabled || len <= 0)
        {
            return;
        }
        m_data[fd].request.append(data, len);
    }

    void append_response(int fd, const char *data, long len)
    {
        if (!m_enabled || len <= 0)
        {
            return;
        }
        m_data[fd].response.append(data, len);
    }

    void dump(int fd, std::ostream &out = std::cout) const
    {
        std::map<int, RawExchange>::const_iterator it = m_data.find(fd);
        if (it == m_data.end())
        {
            return;
        }

        const size_t PREVIEW_LIMIT = 1000;

        out << "\033[1m===== RAW REQUEST fd=" << fd << " (" << it->second.request.size() << " bytes) =====\033[m\n";
        if (it->second.request.size() > PREVIEW_LIMIT)
        {
            out << it->second.request.substr(0, PREVIEW_LIMIT) << "\n\033[2m... [truncated, "
                << (it->second.request.size() - PREVIEW_LIMIT) << " more bytes]\033[m\n";
        }
        else
        {
            out << it->second.request << "\n";
        }

        out << "\033[1m===== RAW RESPONSE fd=" << fd << " (" << it->second.response.size() << " bytes) =====\033[m\n";
        if (it->second.response.size() > PREVIEW_LIMIT)
        {
            out << it->second.response.substr(0, PREVIEW_LIMIT) << "\n\033[2m... [truncated, "
                << (it->second.response.size() - PREVIEW_LIMIT) << " more bytes]\033[m\n";
        }
        else
        {
            out << it->second.response << "\n";
        }
    }

    void dump_to_file(int fd, const std::string &path) const
    {
        std::ofstream file(path.c_str(), std::ios::app);
        if (file.is_open())
        {
            dump(fd, file);
        }
    }

    void dump_all(std::ostream &out = std::cout) const
    {
        for (std::map<int, RawExchange>::const_iterator it = m_data.begin(); it != m_data.end(); ++it)
        {
            dump(it->first, out);
        }
    }

    void erase(int fd)
    {
        m_data.erase(fd);
    }

    const RawExchange *get(int fd) const
    {
        std::map<int, RawExchange>::const_iterator it = m_data.find(fd);
        return it == m_data.end() ? NULL : &it->second;
    }
};

#endif /* UTILS_HPP */
