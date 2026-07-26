#include "../../src/Request/Request.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h> // for unlink()

void print_test_name(const std::string &name)
{
    std::cout << "\n--- Running: " << name << " ---" << std::endl;
}

// Helper function to read the contents of the generated file
std::string read_file_contents(const std::string &filepath)
{
    std::ifstream ifs(filepath.c_str(), std::ios::binary);
    if (!ifs.is_open())
        return "";

    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

void test_unchunked_random_file()
{
    print_test_name("Unchunked Streaming to Random Temp File");
    Request req;

    // 1. Send Headers
    std::string headers = "POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Length: 13\r\n\r\n";
    req.appendDataAndParse(headers.c_str(), headers.length());

    // Ensure we stopped at HEADERS_COMPLETE
    assert(req.getState() == Request::HEADERS_COMPLETE);

    // 2. Trigger random file generation
    assert(req.isReadyForBodyParsing() == true);
    std::string generated_filename = req.getBodyFilename();
    assert(!generated_filename.empty());
    assert(generated_filename.find("/tmp/ws_body_") != std::string::npos);

    // 3. Send Body
    std::string body = "Hello, World!";
    req.appendDataAndParse(body.c_str(), body.length());

    // 4. Verify Complete State
    assert(req.getState() == Request::COMPLETE);

    // 5. Verify File Contents
    std::string file_content = read_file_contents(generated_filename);
    assert(file_content == "Hello, World!");

    // Clean up
    unlink(generated_filename.c_str());

    std::cout << "[OK] Random file generation and unchunked writing passed." << std::endl;
}

void test_chunked_specific_file()
{
    print_test_name("Chunked Streaming to Specific File");
    Request req;

    // 1. Send Headers
    std::string headers = "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n";
    req.appendDataAndParse(headers.c_str(), headers.length());

    assert(req.getState() == Request::HEADERS_COMPLETE);

    // 2. Trigger specific file assignment
    std::string target_file = "/tmp/test_chunked_target.txt";
    assert(req.isReadyForBodyParsing(target_file) == true);
    assert(req.getBodyFilename() == target_file);

    // 3. Send Chunk 1
    std::string chunk1 = "5\r\nChunk\r\n";
    req.appendDataAndParse(chunk1.c_str(), chunk1.length());
    assert(req.getState() == Request::BODY); // Still expecting more

    // 4. Send Chunk 2
    std::string chunk2 = "8\r\n Data...\r\n";
    req.appendDataAndParse(chunk2.c_str(), chunk2.length());

    // 5. Send Terminal Chunk
    std::string terminal = "0\r\n\r\n";
    req.appendDataAndParse(terminal.c_str(), terminal.length());

    // 6. Verify Complete State
    assert(req.getState() == Request::COMPLETE);

    // 7. Verify File Contents
    std::string file_content = read_file_contents(target_file);
    assert(file_content == "Chunk Data..."); // Notice the raw HTTP chunks are gone!

    // Clean up
    unlink(target_file.c_str());

    std::cout << "[OK] Specific file targeting and chunked decoding passed." << std::endl;
}

void test_massive_payload_no_pause()
{
    print_test_name("Massive Payload (Verifying 64KB Limit Removal)");
    Request req;

    size_t massive_size = 150000; // ~150KB (Well over the old 64KB limit)

    // 1. Send Headers
    std::ostringstream oss;
    oss << "POST /massive HTTP/1.1\r\nHost: localhost\r\nContent-Length: " << massive_size << "\r\n\r\n";
    std::string headers = oss.str();
    req.appendDataAndParse(headers.c_str(), headers.length());

    req.isReadyForBodyParsing();
    std::string generated_filename = req.getBodyFilename();

    // 2. Send Massive Body in a single go
    std::string massive_body(massive_size, 'Z');
    req.appendDataAndParse(massive_body.c_str(), massive_body.length());

    // 3. Verify it went straight to COMPLETE without pausing at BODY_CHUNK_READY
    assert(req.getState() == Request::COMPLETE);

    // 4. Verify File Contents
    std::string file_content = read_file_contents(generated_filename);
    assert(file_content.size() == massive_size);
    assert(file_content[0] == 'Z' && file_content[massive_size - 1] == 'Z');

    // Clean up
    unlink(generated_filename.c_str());

    std::cout << "[OK] Massive payloads write continuously without arbitrary pauses." << std::endl;
}

int main()
{
    std::cout << "======================================" << std::endl;
    std::cout << " Request File Buffering Tests         " << std::endl;
    std::cout << "======================================" << std::endl;

    test_unchunked_random_file();
    test_chunked_specific_file();
    test_massive_payload_no_pause();

    std::cout << "\n======================================" << std::endl;
    std::cout << " ALL FILE BUFFERING TESTS PASSED! " << std::endl;
    std::cout << "======================================" << std::endl;

    return 0;
}