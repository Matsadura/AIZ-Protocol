#include "../../src/Request/Request.hpp"
#include <iostream>
#include <map>

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
    VALID CONTENT-LENGTH BODIES
    ============================================================
    */

    {
        Request req;

        req.appendDataAndParse("POST /upload HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: 5\r\n"
                               "\r\n"
                               "HELLO",
                               67);

        printResult("Valid simple body", req, Request::COMPLETE, 0);
    }

    {
        Request req;

        req.appendDataAndParse("POST / HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: 0\r\n"
                               "\r\n",
                               62);

        printResult("Valid zero-length body", req, Request::COMPLETE, 0);
    }

    /*
    ============================================================
    PARTIAL / INCREMENTAL BODY READS
    ============================================================
    */

    {
        Request req;

        req.appendDataAndParse("POST /upload HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: 10\r\n"
                               "\r\n"
                               "HEL",
                               70);

        printResult("Partial body - first recv", req, Request::BODY, 0);

        req.appendDataAndParse("LOWORLD", 7);

        printResult("Partial body - second recv", req, Request::COMPLETE, 0);
    }

    {
        Request req;

        req.appendDataAndParse("POST /upload HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: 5\r\n"
                               "\r\n",
                               62);

        printResult("Headers received, body missing", req, Request::BODY, 0);

        req.appendDataAndParse("HELLO", 5);

        printResult("Body arrives later", req, Request::COMPLETE, 0);
    }

    /*
    ============================================================
    BODY ALREADY INSIDE HEADER BUFFER
    ============================================================
    */

    {
        Request req;

        req.appendDataAndParse("POST / HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: 5\r\n"
                               "\r\nHELLO",
                               67);

        printResult("Body already buffered after headers", req, Request::COMPLETE, 0);
    }

    /*
    ============================================================
    INVALID RFC CASES
    ============================================================
    */

    {
        Request req;

        req.appendDataAndParse("POST / HTTP/1.0\r\n"
                               "Host: localhost\r\n"
                               "\r\n"
                               "HELLO",
                               50);

        /*
        RFC:
        HTTP/1.0 body requires Content-Length
        */

        printResult("HTTP/1.0 body without Content-Length", req, Request::COMPLETE, 0);
    }

    {
        Request req;

        req.appendDataAndParse("POST / HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "\r\n"
                               "HELLO",
                               50);

        /*
        RFC:
        Body without length information
        */

        printResult("HTTP/1.1 body without length", req, Request::COMPLETE, 0);
    }

    {
        Request req;

        req.appendDataAndParse("POST / HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: ABC\r\n"
                               "\r\n",
                               66);

        printResult("Invalid Content-Length", req, Request::ERROR, BAD_REQUEST);
    }

    {
        Request req;

        req.appendDataAndParse("POST / HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: -10\r\n"
                               "\r\n",
                               67);

        printResult("Negative Content-Length", req, Request::ERROR, BAD_REQUEST);
    }

    {
        Request req;

        req.appendDataAndParse("POST / HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: 999999999999999999999999999999\r\n"
                               "\r\n",
                               92);

        printResult("Overflow Content-Length", req, Request::ERROR, BAD_REQUEST);
    }

    {
        Request req;

        req.setMaxBodySize(5);

        req.appendDataAndParse("POST / HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: 10\r\n"
                               "\r\n"
                               "0123456789",
                               72);

        printResult("Payload too large", req, Request::ERROR, PAYLOAD_TOO_LARGE);
    }

    /*
    ============================================================
    REQUEST SMUGGLING PROTECTION
    ============================================================
    */

    {
        Request req;

        req.appendDataAndParse("POST / HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: 5\r\n"
                               "Transfer-Encoding: chunked\r\n"
                               "\r\n",
                               98);

        printResult("Content-Length + Transfer-Encoding", req, Request::ERROR, BAD_REQUEST);
    }

    {
        Request req;

        std::string request = "POST / HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Transfer-Encoding: chunked, chunked\r\n"
                              "\r\n";

        req.appendDataAndParse(request.c_str(), request.size());

        printResult("Duplicate chunked transfer-coding", req, Request::ERROR, BAD_REQUEST);
    }

    {
        Request req;

        std::string request = "POST / HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Transfer-Encoding: chunked, gzip\r\n"
                              "\r\n";

        req.appendDataAndParse(request.c_str(), request.size());

        printResult("Chunked not final transfer-coding", req, Request::ERROR, BAD_REQUEST);
    }

    /*
    ============================================================
    EXTRA DATA AFTER BODY
    ============================================================
    */

    {
        Request req;

        req.appendDataAndParse("POST / HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: 5\r\n"
                               "\r\n"
                               "HELLOWORLD",
                               72);

        /*
        Expected:
        - HELLO goes into body
        - WORLD remains in raw buffer
        */

        printResult("Extra bytes after body", req, Request::COMPLETE, 0);
    }

    return 0;
}
