#include "../../src/Request/Request.hpp"

void test(const std::string &input)
{
    Request req;
    req.appendDataAndParse(input.c_str(), input.size());

    std::cout << "Input: [" << input << "]\n";
    std::cout << "State: " << req.getState() << "\n";
    std::cout << "Method: " << req.getMethod() << "\n";
    std::cout << "URI: " << req.getURI() << "\n";
    std::cout << "Version: " << req.getVersion() << "\n";
    std::cout << "Error Code: " << req.getErrorCode() << "\n";
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
    test("GET /index.html HTTP/1.1\r\nHeader: Value\r\nHost: AnotherValue\r\n");     // Valid request line with
                                                                                     // multiple headers
    test("GET /index.html HTTP/1.1\r\nHeader: Value\r\nHost: AnotherValue\r\n\r\n"); // Valid request line with
                                                                                     // multiple headers and
                                                                                     // proper CRLF
    test(
        "GET /index.html HTTP/1.1\r\nHeader: Value with spaces\r\nHost: AnotherValue with spaces\r\n\r\n"); // Valid
                                                                                                            // request
                                                                                                            // line with
                                                                                                            // headers
                                                                                                            // that
                                                                                                            // contain
                                                                                                            // spaces

    test("GET   /index.html   HTTP/1.1\r\n"); // Valid request line with extra spaces (should be trimmed)
    test("FAKE / HTTP/1.1\r\n");              // Invalid method
    test("PUT / HTTP/1.1\r\n");               // Unsupported method (not GET, POST, or DELETE)
    test("GET /index.html HTTP/1.2\r\n");     // Invalid HTTP version
    test("GET hello HTTP/1.1\r\n");           // Invalid URI (does not start with '/')
    test("GET /he\x01llo HTTP/1.1\r\n");      // Invalid URI (contains control character)
    test("GET / HTTP/1.1\rBAD\n\r\n");        // Invalid request line with CRLF in the middle

    test("GET /hello%20world HTTP/1.1\r\n");  // space (%20)
    test("GET /a%2Fb HTTP/1.1\r\n");          // '/' (%2F)
    test("GET /file%2Etxt HTTP/1.1\r\n");     // '.' (%2E)
    test("GET /%7Euser HTTP/1.1\r\n");        // '~' (%7E)

    test("GET /test% HTTP/1.1\r\n");          // incomplete
    test("GET /test%2 HTTP/1.1\r\n");         // incomplete hex
    test("GET /test%GG HTTP/1.1\r\n");        // invalid hex
    test("GET /test%2Z HTTP/1.1\r\n");        // invalid hex
    test("GET /test%/a HTTP/1.1\r\n");        // broken format

    test("GET /he\x01llo HTTP/1.1\r\n");      // Invalid URI with control character (should be rejected)
    test("GET /test\x7F HTTP/1.1\r\n");       // Invalid URI with control characters (should be rejected)
    test("GET /\x00 HTTP/1.1\r\n"); // Invalid URI with null byte (should be treated as literal character, not rejected)

    test("GET / HTTP/1.1\rBAD\n");  // Invalid request line with CRLF in the middle (should be treated as literal header
                                    // value, not rejected)
    test("GET / HTTP/1.1\r\nInjected: evil\r\n"); // Valid request line but header injection attempt (should be treated
                                                  // as literal header value, not rejected)
    test("GET /test\r\nHTTP/1.1\r\n"); // Valid request line with CRLF in the middle (should be treated as literal URI,
                                       // not rejected)

    test("GET ////////////////////////////////////////// HTTP/1.1\r\n"); // Valid URI with many slashes (should be
                                                                         // accepted, not rejected)
    test("GET /%41%42%43 HTTP/1.1\r\n");                                 // ABC (should be decoded to ABC, not rejected)
    test("GET /%%%% HTTP/1.1\r\n"); // multiple percent signs with no valid hex (should be treated as literal '%', not
                                    // rejected)
    test("GET /%20%20%20 HTTP/1.1\r\n"); // three spaces (should be decoded to spaces, not rejected)
    return 0;
}