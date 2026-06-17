// #include "../../src/Request/Request.hpp"
// #include <iostream>
// #include <map>

// void test(const std::string &input)
// {
//     Request req;
//     req.appendDataAndParse(input.c_str(), input.size());

//     std::cout << "Input: [" << input << "]\n";
//     std::cout << "State: " << req.getState() << "\n";
//     std::cout << "Error Code: " << req.getErrorCode() << "\n";

//     // Assuming getHeaders() returns a std::map<std::string, std::string>
//     std::map<std::string, std::string> headers = req.getHeaders();
//     for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
//     {
//         std::cout << "  " << it->first << ": " << it->second << "\n";
//     }
//     std::cout << "----------------------\n";
// }

// int main()
// {
//     // --- VALID CASES ---
//     test("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");                                     // Standard valid request
//     test("GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n");           // Multiple headers
//     test("GET / HTTP/1.1\r\nHost: localhost\r\nMixed-CASE-Header: Value\r\n\r\n");         // Should lowercase keys
//     test("GET / HTTP/1.1\r\nHost: localhost\r\nSpaces:    value with spaces    \r\n\r\n"); // Value trimming
//     test("GET / HTTP/1.0\r\n\r\n"); // Valid: HTTP/1.0 does not require Host header

//     // --- PARTIAL READS (State should remain HEADERS, not BODY) ---
//     test("GET / HTTP/1.1\r\nHost: localhost\r\n"); // Missing final \r\n, waiting for more data
//     test("GET / HTTP/1.1\r\nHost: local");         // Half-written header, waiting for CRLF

//     // --- HTTP/1.1 HOST REQUIREMENT (RFC 7230) ---
//     test("GET / HTTP/1.1\r\nConnection: close\r\n\r\n"); // Invalid: Missing Host
//     test("GET / HTTP/1.1\r\nHost:\r\n\r\n");             // Invalid: Empty Host value
//     test("GET / HTTP/1.1\r\nHost:    \r\n\r\n");         // Invalid: Host value is just whitespace

//     // --- WHITESPACE / SMUGGLING PREVENTION (RFC 7230) ---
//     test("GET / HTTP/1.1\r\nHost : localhost\r\n\r\n");  // Invalid: Space before colon
//     test("GET / HTTP/1.1\r\nHost\t: localhost\r\n\r\n"); // Invalid: Tab before colon

//     // --- DUPLICATE HEADERS ---
//     test("GET / HTTP/1.1\r\nHost: localhost\r\nAccept: text/html\r\nAccept: text/plain\r\n\r\n"); // Valid: Should merge
//                                                                                                   // with comma
//     test("GET / HTTP/1.1\r\nHost: localhost\r\nHost: google.com\r\n\r\n"); // Invalid: Cannot duplicate Host
//     test("GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nContent-Length: 5\r\n\r\n"); // Invalid: Cannot
//                                                                                                  // duplicate
//                                                                                                  // Content-Length

//     // --- MALFORMED HEADERS ---
//     test("GET / HTTP/1.1\r\nHost: localhost\r\nBadHeaderNoColon\r\n\r\n"); // Invalid: Missing colon
//     test("GET / HTTP/1.1\r\n: empty key\r\n\r\n");                         // Invalid: Empty key

//     // --- CONTENT-LENGTH VALIDATION ---
//     test("GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 1024\r\n\r\n"); // Valid numeric Content-Length
//     test("GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10A4\r\n\r\n"); // Invalid: Contains letters
//     test("GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: -10\r\n\r\n");  // Invalid: Negative number

//     // --- OVERSIZED HEADERS ---
//     std::string giantHeader = "GET / HTTP/1.1\r\nHost: localhost\r\nBig: ";
//     giantHeader.append(8193, 'A'); // Over 8192 byte limit
//     giantHeader += "\r\n\r\n";
//     test(giantHeader);             // Invalid: Exceeds size limit

//     return 0;
// }