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

// Helper to create a dummy executable file
void create_executable_file(const std::string &filepath, const std::string &content)
{
    std::ofstream ofs(filepath.c_str(), std::ios::binary);
    ofs << content;
    ofs.close();

    // Give it executable permissions
    chmod(filepath.c_str(), 0755);
}

void test_cgi_execution_with_file_body()
{
    print_test_name("CGI Execution (File STDIN Mapping)");

    // 1. Create a dummy CGI shell script
    // This script reads from STDIN and echoes it back out, proving the file mapping worked.
    std::string script_path    = "/tmp/ws_test_script.sh";
    std::string script_content = "#!/bin/sh\n"
                                 "read BODY_INPUT\n"
                                 "echo \"Content-Type: text/plain\"\n"
                                 "echo \"Status: 200 OK\"\n"
                                 "echo \"\"\n"
                                 "echo \"CGI Output: $BODY_INPUT\"\n"
                                 "echo \"METHOD: $REQUEST_METHOD\"\n";

    create_executable_file(script_path, script_content);

    // 2. Mock a Request object
    Request     req;
    std::string request_data = "POST /cgi-bin/test.sh HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: 12\r\n\r\n";
    req.appendDataAndParse(request_data.c_str(), request_data.length());

    // 3. Provide the file body directly (This triggers the file buffering)
    std::string body_file = "/tmp/ws_test_body.txt";
    req.isReadyForBodyParsing(body_file);

    std::string body_payload = "Hello Server";
    req.appendDataAndParse(body_payload.c_str(), body_payload.length());
    assert(req.getState() == Request::COMPLETE);

    // 4. Setup Ali's CGI Metadata
    CgiMetaData meta;
    meta.is_cgi           = true;
    meta.script_path      = script_path;
    meta.interpreter_path = "/bin/sh";

    // 5. Execute CGI
    CGI cgi;
    cgi.execute(req, meta);

    // 6. Read the output from the CGI script
    int out_fd = cgi.getOutFd();
    assert(out_fd != -1);

    // Sleep slightly to let the child process spin up and write to the pipe
    // (This replaces the epoll_wait loop for the sake of a unit test)
    usleep(150000);

    char    buffer[1024];
    ssize_t bytes_read = read(out_fd, buffer, sizeof(buffer) - 1);
    assert(bytes_read > 0);
    buffer[bytes_read] = '\0';

    std::string cgi_output(buffer);

    // 7. Validate output: Ensure STDIN read the payload and ENV vars mapped properly
    assert(cgi_output.find("Content-Type: text/plain") != std::string::npos);
    assert(cgi_output.find("Status: 200 OK") != std::string::npos);
    assert(cgi_output.find("CGI Output: Hello Server") != std::string::npos);
    assert(cgi_output.find("METHOD: POST") != std::string::npos);

    // Clean up
    cgi.waitAndClean();
    unlink(script_path.c_str());
    unlink(body_file.c_str());

    std::cout << "[OK] CGI successfully read from file STDIN and outputted to pipe." << std::endl;
}

int main()
{
    std::cout << "======================================" << std::endl;
    std::cout << "     CGI Execution Tests              " << std::endl;
    std::cout << "======================================" << std::endl;

    test_cgi_execution_with_file_body();

    std::cout << "\n======================================" << std::endl;
    std::cout << " ALL CGI EXECUTION TESTS PASSED!      " << std::endl;
    std::cout << "======================================" << std::endl;

    return 0;
}