#include "../../src/Request/Request.hpp"
#include "../../src/Response/Response.hpp"

void test(const std::string &input)
{
    Request req;
    req.appendDataAndParse(input.c_str(), input.size());
    Response res(req);
    res.process();
    std::cout << "Response: [" << res.getResponseBuffer() << "]\n";
}

int main()
{
    // test("GET /index.html HTTP/1.1\r\n");                  // Valid request
    // test("POST /submit HTTP/1.0\r\n");                     // Valid request with different method and version
    test("INVALID REQUEST\r\n");                           // Invalid request line
    // test("GET /index.html HTTP/1.1");                      // Missing CRLF
    // test("GET /index.html HTTP/1.1\r\nExtraData");         // Valid request line but extra data after CRLF
    // test("GET /index.html HTTP/1.1\r\n\r\n");              // Valid request line with empty headers
    // test("GET /index.html HTTP/1.1\r\nHeader: Value\r\n"); // Valid request line with one header
    // test("GET /index.html HTTP/1.1\r\nHeader: Value\r\nHost: AnotherValue\r\n");     // Valid request line with
                                                                                     // multiple headers
}