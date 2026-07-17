#include <string>

/**
 * Use @m_data_type to decide what to do, because @m_data can be multiple things
 */
class RouterResult
{
  public:
    enum Type
    {
        STRING_BUFFER, // @m_data is the content that will be served
        FILE_PATH,     // @m_data can be a valid file_path or empty, if its empty means use your default
        REDIRECTION,   // @m_data is the location for the redirection
    };

    int         m_http_code;
    std::string m_data;
    Type        m_data_type;

    RouterResult(int http_code, const std::string &data, Type data_type) :
        m_http_code(http_code), m_data(data), m_data_type(data_type)
    {
    }

    bool operator==(const RouterResult &other)
    {
        return m_http_code == other.m_http_code && m_data == other.m_data && m_data_type == other.m_data_type;
    }
};
