#include "../../src/CGI/CGIResponse.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

void print_test_name(const std::string &name)
{
    std::cout << "\n--- Running: " << name << " ---" << std::endl;
}

void test_basic_streaming()
{
    print_test_name("Basic Header and Body Streaming");
    CGIResponse cgi;

    // 1. Send CGI headers and a small body
    std::string mock_output = "Content-type: text/html\r\nStatus: 200 OK\r\n\r\n<html>Hello</html>";
    cgi.appendCgiData(mock_output.c_str(), mock_output.length());

    // 2. Extract the output
    const std::vector<char> &buffer = cgi.getBodyBuffer();
    std::string              output(buffer.begin(), buffer.end());

    // 3. Verify HTTP translation
    assert(output.find("HTTP/1.1 200 OK\r\n") != std::string::npos);
    assert(output.find("Content-Type: text/html\r\n") != std::string::npos);
    assert(output.find("Transfer-encoding: chunked\r\n") != std::string::npos);

    // 4. Verify HTTP chunk formatting (12 bytes in hex is 'c')
    assert(output.find("12\r\n<html>Hello</html>\r\n") != std::string::npos);

    std::cout << "[OK] Basic streaming passed." << std::endl;
}

void test_local_redirect()
{
    print_test_name("Local Redirect Detection");
    CGIResponse cgi;

    std::string mock_output = "Location: /dashboard.php\r\n\r\n";
    cgi.appendCgiData(mock_output.c_str(), mock_output.length());

    assert(cgi.isLocalRedirect() == true);
    assert(cgi.getErrorCode() == 0);

    std::cout << "[OK] Local redirect parsed correctly." << std::endl;
}

void test_client_redirect()
{
    print_test_name("Client Redirect Detection");
    CGIResponse cgi;

    std::string mock_output = "Location: http://example.com/\r\n\r\n";
    cgi.appendCgiData(mock_output.c_str(), mock_output.length());

    assert(cgi.isLocalRedirect() == false);

    const std::vector<char> &buffer = cgi.getBodyBuffer();
    std::string              output(buffer.begin(), buffer.end());
    assert(output.find("HTTP/1.1 302 Found\r\n") != std::string::npos);

    std::cout << "[OK] Client redirect (302 Found) parsed correctly." << std::endl;
}

void test_invalid_header_space_before_colon()
{
    print_test_name("RFC Violation: Space Before Colon");
    CGIResponse cgi;

    // The space before the colon is strictly forbidden by the RFC
    std::string mock_output = "Content-type : text/html\r\n\r\n";
    cgi.appendCgiData(mock_output.c_str(), mock_output.length());

    assert(cgi.getErrorCode() == 502);
    assert(cgi.getCgiState() == CGIResponse::CGI_COMPLETE);

    const std::vector<char> &buffer = cgi.getBodyBuffer();
    std::string              output(buffer.begin(), buffer.end());
    assert(output.find("502 Error") != std::string::npos);

    std::cout << "[OK] Space before colon properly rejected (502)." << std::endl;
}

void test_continuation_lines_rejected()
{
    print_test_name("RFC Violation: Continuation Lines");
    CGIResponse cgi;

    // A line starting with a space is a continuation line, forbidden in CGI/1.1
    std::string mock_output = "Content-Type: text/html\r\n  charset=UTF-8\r\n\r\n";
    cgi.appendCgiData(mock_output.c_str(), mock_output.length());

    assert(cgi.getErrorCode() == 502);

    std::cout << "[OK] Continuation lines properly rejected (502)." << std::endl;
}

void test_buffer_full_backpressure()
{
    print_test_name("Backpressure: isBufferFull()");
    CGIResponse cgi;

    // 1. Send valid headers to transition to STREAMING state
    std::string mock_output = "Content-Type: text/plain\r\n\r\n";
    cgi.appendCgiData(mock_output.c_str(), mock_output.length());

    // 2. Ensure buffer is not full initially
    assert(cgi.isBufferFull() == false);

    // 3. Flood the buffer with 65KB of data
    std::string massive_chunk(66000, 'A');
    cgi.appendCgiData(massive_chunk.c_str(), massive_chunk.length());

    // 4. Verify the backpressure flag triggers
    assert(cgi.isBufferFull() == true);

    // 5. Simulate Ali consuming the data over the socket
    cgi.consumeBodyChunk(66000);

    // 6. Verify the backpressure flag resets
    assert(cgi.isBufferFull() == false);

    std::cout << "[OK] Buffer capacity and backpressure correctly managed." << std::endl;
}

void test_terminal_chunk()
{
    print_test_name("Terminal Chunk Generation");
    CGIResponse cgi;

    // Send valid headers to transition to STREAMING state
    std::string mock_output = "Content-Type: text/plain\r\n\r\n";
    cgi.appendCgiData(mock_output.c_str(), mock_output.length());

    // Clear the HTTP headers from the buffer to isolate the terminal chunk test
    cgi.consumeBodyChunk(cgi.getBodyBuffer().size());

    // Simulate EOF from the CGI pipe
    cgi.appendTerminalChunk();

    const std::vector<char> &buffer = cgi.getBodyBuffer();
    std::string              output(buffer.begin(), buffer.end());

    assert(cgi.getCgiState() == CGIResponse::CGI_COMPLETE);
    assert(output == "0\r\n\r\n");

    std::cout << "[OK] Terminal chunk successfully appended." << std::endl;
}

int main()
{
    std::cout << "======================================" << std::endl;
    std::cout << "     CGIResponse Unit Tests           " << std::endl;
    std::cout << "======================================" << std::endl;

    test_basic_streaming();
    test_local_redirect();
    test_client_redirect();
    test_invalid_header_space_before_colon();
    test_continuation_lines_rejected();
    test_buffer_full_backpressure();
    test_terminal_chunk();

    std::cout << "\n======================================" << std::endl;
    std::cout << " ALL TESTS PASSED SUCCESSFULLY! " << std::endl;
    std::cout << "======================================" << std::endl;

    return 0;
}