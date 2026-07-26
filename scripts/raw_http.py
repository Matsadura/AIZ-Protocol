#!/usr/bin/env python3
import socket

def send_raw_request(host, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))

    # Generate a body larger than 64KB (70,000 bytes)
    body_length = 70000
    body = "X" * body_length

    # Formulate the HTTP POST request headers
    request_headers = (
        "POST /test.cgi HTTP/1.1\r\n"
        f"Host: {host}:{port}\r\n"
        "Content-Type: text/plain\r\n"
        f"Content-Length: {body_length}\r\n"
        "\r\n"
    )
    
    # Concatenate headers and body
    request = request_headers + body

    # Send the request
    s.sendall(request.encode('utf-8'))
    print("=== RAW REQUEST HEADERS ===")
    print(request_headers)

    # Receive the response in a loop to ensure the full payload is captured
    response = b""
    while True:
        chunk = s.recv(4096)
        print("recieving\n")
        if not chunk:
            break
        response += chunk

    print("=== RAW RESPONSE ===")
    print(response.decode('utf-8', errors='replace'))
    print("Recieved data size:", len(response))

    s.close()

if __name__ == "__main__":
    send_raw_request("localhost", 8080)
