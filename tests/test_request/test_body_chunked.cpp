#include "../../src/Request/Request.hpp"
#include <iostream>
#include <map>
#include <string>

static void printResult(const std::string &name, Request &req, int expected_state, int expected_error)
{
    std::cout << "=== " << name << " ===\n";

    std::cout << "Expected State: " << expected_state << "\n";
    std::cout << "Actual State:   " << req.getState() << "\n";

    std::cout << "Expected Error: " << expected_error << "\n";
    std::cout << "Actual Error:   " << req.getErrorCode() << "\n";

    std::cout << "Body Size: " << req.getBody().size() << "\n";

    std::map<std::string, std::string> headers = req.getHeaders();

    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
    {
        std::cout << it->first << ": " << it->second << "\n";
    }

    if (req.getState() == expected_state && req.getErrorCode() == expected_error)
    {
        std::cout << "[PASS]\n";
    }
    else
    {
        std::cout << "[FAIL]\n";
    }

    std::cout << "-----------------------------\n";
}

int main()
{
    /*
    ============================================================
    VALID CHUNKED BODIES
    ============================================================
    */

    {
        Request req;
        std::string payload = "POST /upload HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Transfer-Encoding: chunked\r\n"
                              "\r\n"
                              "5\r\n"
                              "HELLO\r\n"
                              "0\r\n"
                              "\r\n";

        req.appendDataAndParse(payload.c_str(), payload.size());
        printResult("Valid simple chunked body", req, Request::COMPLETE, 0);
    }

    {
        Request req;
        std::string payload = "POST / HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Transfer-Encoding: chunked\r\n"
                              "\r\n"
                              "0\r\n"
                              "\r\n";

        req.appendDataAndParse(payload.c_str(), payload.size());
        printResult("Valid zero-length chunked body", req, Request::COMPLETE, 0);
    }

    /*
    ============================================================
    PARTIAL / INCREMENTAL CHUNK READS (The Tricky Stuff)
    ============================================================
    */

    {
        Request req;
        std::string part1 = "POST /upload HTTP/1.1\r\n"
                            "Host: localhost\r\n"
                            "Transfer-Encoding: chunked\r\n"
                            "\r\n"
                            "A\r\n"
                            "HELLOWO"; // 7 bytes, 3 remaining

        req.appendDataAndParse(part1.c_str(), part1.size());
        printResult("Partial chunk data - first recv", req, Request::BODY, 0);

        std::string part2 = "RLD\r\n0\r\n\r\n";
        req.appendDataAndParse(part2.c_str(), part2.size());
        printResult("Partial chunk data - second recv", req, Request::COMPLETE, 0);
    }

    {
        Request req;
        std::string part1 = "POST /upload HTTP/1.1\r\n"
                            "Host: localhost\r\n"
                            "Transfer-Encoding: chunked\r\n"
                            "\r\n"
                            "5\r"; // Partial size line

        req.appendDataAndParse(part1.c_str(), part1.size());
        printResult("Partial chunk size line (\r split)", req, Request::BODY, 0);

        std::string part2 = "\nHELLO\r\n0\r\n\r\n";
        req.appendDataAndParse(part2.c_str(), part2.size());
        printResult("Rest of chunk size and body", req, Request::COMPLETE, 0);
    }

    /*
    ============================================================
    CHUNK EXTENSIONS & TRAILERS (RFC Compliance)
    ============================================================
    */

    {
        Request req;
        std::string payload = "POST /upload HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Transfer-Encoding: chunked\r\n"
                              "\r\n"
                              "5;foo=bar;baz=qux\r\n" // Extensions should be ignored
                              "HELLO\r\n"
                              "0\r\n"
                              "\r\n";

        req.appendDataAndParse(payload.c_str(), payload.size());
        printResult("Chunked body with extensions", req, Request::COMPLETE, 0);
    }

    {
        Request req;
        std::string part1 = "POST /upload HTTP/1.1\r\n"
                            "Host: localhost\r\n"
                            "Transfer-Encoding: chunked\r\n"
                            "\r\n"
                            "5\r\n"
                            "HELLO\r\n"
                            "0\r\n"
                            "X-Trailer-1: Value1\r\n"
                            "X-Trailer-2: Value2\r\n"; 

        req.appendDataAndParse(part1.c_str(), part1.size());
        printResult("Chunked body inside trailer", req, Request::BODY, 0); // Wait for \r\n

        std::string part2 = "\r\n"; // Final empty line ends the request
        req.appendDataAndParse(part2.c_str(), part2.size());
        printResult("Chunked body trailer finished", req, Request::COMPLETE, 0);
    }

    /*
    ============================================================
    INVALID RFC CASES & ERRORS
    ============================================================
    */

    {
        Request req;
        std::string payload = "POST /upload HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Transfer-Encoding: chunked\r\n"
                              "\r\n"
                              "5\r\n"
                              "HELLOABCDEF\r\n"; // Missing CRLF right after 5 bytes

        req.appendDataAndParse(payload.c_str(), payload.size());
        printResult("Strict CRLF check after chunk data", req, Request::ERROR, BAD_REQUEST);
    }

    {
        Request req;
        std::string payload = "POST /upload HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Transfer-Encoding: chunked\r\n"
                              "\r\n"
                              "G\r\n" // Invalid Hex
                              "HELLO\r\n"
                              "0\r\n"
                              "\r\n";

        req.appendDataAndParse(payload.c_str(), payload.size());
        printResult("Invalid hex character in chunk size", req, Request::ERROR, BAD_REQUEST);
    }

    {
        Request req;
        std::string payload = "POST /upload HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Transfer-Encoding: chunked\r\n"
                              "\r\n"
                              "FFFFFFFFFFFFFFFFFFFFFFFFFFFF\r\n" // Overflow size
                              "\r\n";

        req.appendDataAndParse(payload.c_str(), payload.size());
        printResult("Overflow chunk size (strtoul limit)", req, Request::ERROR, BAD_REQUEST);
    }

    /*
    ============================================================
    MAX BODY SIZE ENFORCEMENT
    ============================================================
    */

    {
        Request req;
        req.setMaxBodySize(8); // Set max size limit

        std::string part1 = "POST /upload HTTP/1.1\r\n"
                            "Host: localhost\r\n"
                            "Transfer-Encoding: chunked\r\n"
                            "\r\n"
                            "5\r\n"
                            "HELLO\r\n";

        req.appendDataAndParse(part1.c_str(), part1.size());
        printResult("Chunk 1 (Under Limit)", req, Request::BODY, 0);

        std::string part2 = "5\r\n"
                            "WORLD\r\n" // This pushes total to 10 > 8
                            "0\r\n"
                            "\r\n";

        req.appendDataAndParse(part2.c_str(), part2.size());
        printResult("Chunk 2 (Exceeds Limit)", req, Request::ERROR, PAYLOAD_TOO_LARGE);
    }

    /*
    ============================================================
    MULTIPLE CHUNKS WITH EXTRA DATA AFTERWARD
    ============================================================
    */

    {
        Request req;
        std::string payload = "POST /upload HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Transfer-Encoding: chunked\r\n"
                              "\r\n"
                              "3\r\n"
                              "HEL\r\n"
                              "2\r\n"
                              "LO\r\n"
                              "0\r\n"
                              "\r\n"
                              "EXTRA_PIPELINED_REQUEST_DATA";

        req.appendDataAndParse(payload.c_str(), payload.size());
        printResult("Multiple chunks + extra pipeline data", req, Request::COMPLETE, 0);
        
        // At this point, req.m_raw_buffer should contain "EXTRA_PIPELINED_REQUEST_DATA"
    }

    return 0;
}