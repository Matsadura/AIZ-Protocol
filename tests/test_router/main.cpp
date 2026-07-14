#include "../../src/Router/Router.hpp"
#include "../../src/config_file_parser/parser/configfile.hpp"

static int g_id   = 0;
static int g_fail = 0;

static const char *type_str(RouterResult::Type t)
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

void expect_router_result(const s_Server &server, const std::string &uri, const RouterResult &expected)
{
    int          id     = ++g_id;
    RouterResult actual = router_get_resource(server, uri);

    if (!(actual == expected))
    {
        ++g_fail;
        std::cout << "FAIL " << id << " " << uri << "\n"
                  << "  got  " << actual.m_http_code << " " << type_str(actual.m_data_type) << " \"" << actual.m_data
                  << "\"\n"
                  << "  want " << expected.m_http_code << " " << type_str(expected.m_data_type) << " \""
                  << expected.m_data << "\"\n";
    }
}

// .
// └── www
//     ├── html
//     │   ├── about.html
//     │   ├── emptydir
//     │   ├── index.html
//     │   ├── normaldir
//     │   │   └── file1.txt
//     │   ├── passwd-link
//     │   └── secret.txt
//     ├── lockeddir/ (Permission denied)
#define HTML_ROOT "/tmp/webserv_test/www/html/"

int main()
{
    ConfigFile config("configfile/router.conf");
    s_Server   main_site = config.getConfig().front();

    // --- plain file serving ---
    expect_router_result(main_site, "/index.html", RouterResult(200, HTML_ROOT "index.html", RouterResult::FILE_PATH));

    expect_router_result(main_site, "/about.html", RouterResult(200, HTML_ROOT "about.html", RouterResult::FILE_PATH));

    // --- directory handling ---
    expect_router_result(main_site, "/indexed_dir/",
                         RouterResult(200, generate_directory_listing("/tmp/webserv_test/www/", "/indexed_dir/"),
                                      RouterResult::STRING_BUFFER));

    expect_router_result(main_site, "/indexed_dir/html/normaldir/",
                         RouterResult(200,
                                      generate_directory_listing("/tmp/webserv_test/www/html/normaldir/",
                                                                 "/indexed_dir/html/normaldir/"),
                                      RouterResult::STRING_BUFFER));

    expect_router_result(main_site, "/indexed_dir", RouterResult(301, "/indexed_dir/", RouterResult::REDIRECTION));

    expect_router_result(main_site, "/noindex/",
                         RouterResult(403, get_default_page(main_site, 403), RouterResult::ERROR_PAGE));

    expect_router_result(main_site, "/noindex/index.php",
                         RouterResult(200, "/tmp/webserv_test/www/noindexdir/index.php", RouterResult::FILE_PATH));

    // --- permissions ---
    expect_router_result(main_site, "/locked-listing/",
                         RouterResult(403, get_default_page(main_site, 403), RouterResult::ERROR_PAGE));
    expect_router_result(main_site, "/secret.txt",
                         RouterResult(403, get_default_page(main_site, 403), RouterResult::ERROR_PAGE));
    expect_router_result(main_site, "/dir_listing_off/",
                         RouterResult(200, "/tmp/webserv_test/www/noindexdir/index.php", RouterResult::FILE_PATH));

    // --- missing things ---
    expect_router_result(main_site, "/does-not-exist.txt",
                         RouterResult(404, get_default_page(main_site, 404), RouterResult::ERROR_PAGE));

    expect_router_result(main_site, "/noroot/anything",
                         RouterResult(404, get_default_page(main_site, 404), RouterResult::ERROR_PAGE));

    expect_router_result(main_site, "/noroot/",
                         RouterResult(404, get_default_page(main_site, 404), RouterResult::ERROR_PAGE));

    // --- redirection directive ---
    expect_router_result(main_site, "/old-page", RouterResult(301, "/new-page", RouterResult::REDIRECTION));

    expect_router_result(main_site, "/old-page/extra/bits",
                         RouterResult(301, "/new-page/extra/bits", RouterResult::REDIRECTION));

    // --- real system files, no fixture needed ---
    expect_router_result(main_site, "/sysfiles/hostname", RouterResult(200, "/etc/hostname", RouterResult::FILE_PATH));

    expect_router_result(main_site, "/sysfiles/shadow",
                         RouterResult(403, get_default_page(main_site, 403), RouterResult::ERROR_PAGE));

    expect_router_result(main_site, "/sysfiles/nope",
                         RouterResult(404, get_default_page(main_site, 404), RouterResult::ERROR_PAGE));

    std::cout << "\n\nRan " << g_id << " tests: ";
    if (g_fail != 0)
    {
        std::cout << "FAILED (failures=" << g_fail << ")\n";
        return 1;
    }
    else
    {
        std::cout << "SUCCEED\n";
    }
    return g_fail == 0 ? 0 : 1;
}
