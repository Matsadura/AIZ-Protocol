// THIS IS AI GENERATED CODE
// This file is a temporary test harness for the HTTP request parser.

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

#define PORT 8080
#define BUFFER_SIZE 30000

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address = {};
    int opt                    = 1;
    int addrlen                = sizeof(address);
    char buffer[BUFFER_SIZE]   = {0};

    // 1. Create a raw socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Socket failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 2. Allow port reuse (so you can restart the test quickly)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        std::cerr << "Setsockopt failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 3. Bind to port 8080
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 4. Start listening
    if (listen(server_fd, 3) < 0)
    {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Dummy server listening on port " << PORT << "..." << std::endl;
    std::cout << "Go to http://localhost:8080 in your browser." << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;

    // 5. Infinite loop to accept connections one by one
    while (true)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            std::cerr << "Accept failed" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Clear the buffer
        std::memset(buffer, 0, BUFFER_SIZE);

        // Read the incoming HTTP request
        ssize_t valread = read(new_socket, buffer, BUFFER_SIZE - 1);
        if (valread > 0)
        {
            std::cout << "=== RECEIVED RAW REQUEST ===" << std::endl;
            std::cout << buffer << std::endl;
            std::cout << "============================" << std::endl;

            // ---------------------------------------------------------
            // THIS IS WHERE YOU TEST YOUR PARSER:
            // HttpRequest request;
            // request.feedData(buffer, valread);
            // std::cout << "Parsed Method: " << request.getMethod() << std::endl;
            // ---------------------------------------------------------
        }

        // Send a valid, hardcoded HTTP response so the browser doesn't hang
        std::string response =
            "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 21\r\n\r\nParser Test Harness!\n";
        write(new_socket, response.c_str(), response.length());

        // Close the connection
        close(new_socket);
    }

    return 0;
}