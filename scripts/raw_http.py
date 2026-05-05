#!/bin/env python3
import socket

def send_raw_request(host, port):
    # Create a TCP socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))

    # Formulate the exact raw bytes, explicitly using \r\n
    request = (
        # "GET /?param=value HTTP/1.0\r\n"
        r"GET /\x00 HTTP/1.1\r\n"
        f"Host : {"A"*10}\r\n"
        f"blabla123 : {"B"*10}\r\n"
        "\r\n"
    )

    # Send the request
    s.sendall(request.encode('utf-8'))

    # Receive the response (up to 4096 bytes)
    response = s.recv(4096)
    print("=== RAW REQUEST ===")
    print(request)
    print("=== RAW RESPONSE ===")
    print(response.decode('utf-8', errors='replace'))

    s.close()

if __name__ == "__main__":
    send_raw_request("localhost", 8080) # Local server test
    # send_raw_request("54.159.165.144", 80) # Remote server test (uncomment to use)
