#include "../../src/Router/Router.hpp"
#include "../../src/config_file_parser/parser/configfile.hpp"
#include <iostream>
#include <string>

static const char *kind_name(RouterResult::Type t)
{
    switch (t)
    {
        case RouterResult::STRING_BUFFER:
            return "STRING_BUFFER";
        case RouterResult::FILE_PATH:
            return "FILE_PATH";
        case RouterResult::ERROR_PAGE:
            return "ERROR_PAGE";
        case RouterResult::REDIRECTION:
            return "REDIRECTION";
    }
    return "?";
}

static void run(const s_Server &server, const std::string &uri)
{
    RouterResult r = router_get_resource(server, uri);
    std::cout << uri << " -> code=" << r.m_http_code << " type=" << kind_name(r.m_data_type)
              << " data=" << (r.m_data_type == RouterResult::STRING_BUFFER ? "<buffer>" : r.m_data) << "\n";
}

int main()
{
    ConfigFile config("configfile/test.conf");
    s_Server   server = config.getConfig()[0];

    const char *cases[] = {
        "/",             // 200 FILE_PATH, served directly
        "/gdb/gdbinit",  // 200, dir + index.html
        "/nothing-here", // 200, dir + index.html (different location)
        "/forbid/"       // 403
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); ++i)
        run(server, cases[i]);
}
