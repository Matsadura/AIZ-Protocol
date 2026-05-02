#include "../request.hpp"

void test(const std::string &input)
{
    Request req;
    req.appendData(input.c_str(), input.size());

    req.parseRequestLine();

    std::cout << "Input: [" << input << "]\n";
    std::cout << "State: " << req.getState() << "\n";
    std::cout << "Method: " << req.getMethod() << "\n";
    std::cout << "URI: " << req.getURI() << "\n";
    std::cout << "Version: " << req.getVersion() << "\n";
    std::cout << "----------------------\n";
}

int main()
{
    test("GET /index.html HTTP/1.1\r\n");                  // Valid request
    test("POST /submit HTTP/1.0\r\n");                     // Valid request with different method and version
    test("INVALID REQUEST\r\n");                           // Invalid request line
    test("GET /index.html HTTP/1.1");                      // Missing CRLF
    test("GET /index.html HTTP/1.1\r\nExtraData");         // Valid request line but extra data after CRLF
    test("GET /index.html HTTP/1.1\r\n\r\n");              // Valid request line with empty headers
    test("GET /index.html HTTP/1.1\r\nHeader: Value\r\n"); // Valid request line with one header
    test("GET /index.html HTTP/1.1\r\nHeader: Value\r\nAnotherHeader: AnotherValue\r\n");     // Valid request line with
                                                                                              // multiple headers
    test("GET /index.html HTTP/1.1\r\nHeader: Value\r\nAnotherHeader: AnotherValue\r\n\r\n"); // Valid request line with
                                                                                              // multiple headers and
                                                                                              // proper CRLF
    test(
        "GET /index.html HTTP/1.1\r\nHeader: Value with spaces\r\nAnotherHeader: AnotherValue with spaces\r\n\r\n"); // Valid request line with headers that contain spaces

    return 0;
}