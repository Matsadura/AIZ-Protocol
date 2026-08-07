#include "../../src/CGI/CGI.hpp"
#include "../../src/Request/Request.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

void print_test_name(const std::string &name)
{
    std::cout << "\n--- Running: " << name << " ---" << std::endl;
}

void create_file(const std::string &filepath, const std::string &content, bool executable)
{
    std::ofstream ofs(filepath.c_str(), std::ios::binary);
    ofs << content;
    ofs.close();

    if (executable)
        chmod(filepath.c_str(), 0755);
}

void test_cgi_working_directory()
{
    print_test_name("CGI Execution (Relative Path / Chdir Resolution)");

    // 1. Setup a specific directory for the CGI script
    std::string test_dir = "/tmp/ws_cgi_dir_test";
    mkdir(test_dir.c_str(), 0777);

    // 2. Create a data file inside that directory
    std::string data_file = test_dir + "/secret_data.txt";
    create_file(data_file, "SUCCESS_CHDIR_RELATIVE_READ", false);

    // 3. Create a CGI script that reads the data file using a RELATIVE path
    std::string script_path    = test_dir + "/relative_read.sh";
    std::string script_content = "#!/bin/sh\n"
                                 "echo \"Content-Type: text/plain\"\n"
                                 "echo \"Status: 200 OK\"\n"
                                 "echo \"\"\n"
                                 "cat secret_data.txt\n"; // <-- Notice this is a relative path!

    create_file(script_path, script_content, true);

    // 4. Change the parent program's working directory to somewhere else entirely
    // If chdir() isn't working in the child, the script will try to look for
    // /var/tmp/secret_data.txt and fail.
    chdir("/var/tmp");

    // 5. Mock a Request object (Standard GET request, no body)
    Request     req;
    std::string request_data = "GET /cgi-bin/relative_read.sh HTTP/1.1\r\n"
                               "Host: localhost\r\n\r\n";
    req.appendDataAndParse(request_data.c_str(), request_data.length());
    assert(req.getState() == Request::COMPLETE);

    // 6. Setup Ali's CGI Metadata
    CgiMetaData meta;
    meta.is_cgi           = true;
    meta.script_path      = script_path;
    meta.interpreter_path = "/bin/sh";

    // 7. Execute CGI
    CGI cgi;
    cgi.execute(req, meta);

    // 8. Read the output from the CGI script
    int out_fd = cgi.getOutFd();
    assert(out_fd != -1);

    usleep(150000); // Sleep slightly to let the child process spin up and read the file

    char    buffer[1024];
    ssize_t bytes_read = read(out_fd, buffer, sizeof(buffer) - 1);
    assert(bytes_read > 0);
    buffer[bytes_read] = '\0';

    std::string cgi_output(buffer);

    // 9. Validate output: Ensure the script successfully found and read the relative file
    assert(cgi_output.find("Content-Type: text/plain") != std::string::npos);
    assert(cgi_output.find("Status: 200 OK") != std::string::npos);

    if (cgi_output.find("SUCCESS_CHDIR_RELATIVE_READ") != std::string::npos)
    {
        std::cout << "[OK] CGI successfully changed directory and resolved the relative path!" << std::endl;
    }
    else
    {
        std::cerr << "[FAIL] CGI failed to resolve relative path. Output was:\n" << cgi_output << std::endl;
        assert(false);
    }

    // Clean up
    cgi.waitAndClean();
    unlink(script_path.c_str());
    unlink(data_file.c_str());
    rmdir(test_dir.c_str());
}

int main()
{
    std::cout << "======================================" << std::endl;
    std::cout << "     CGI Directory Path Tests         " << std::endl;
    std::cout << "======================================" << std::endl;

    test_cgi_working_directory();

    std::cout << "\n======================================" << std::endl;
    std::cout << " ALL PATH TESTS PASSED!               " << std::endl;
    std::cout << "======================================" << std::endl;

    return 0;
}