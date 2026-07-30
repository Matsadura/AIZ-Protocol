#!/usr/bin/env python3
"""
echo_body.py — minimal CGI script to check what body the server hands you.

Reads CONTENT_LENGTH bytes from stdin (the request body) and echoes them
straight back in the response, along with a couple of useful headers.

Deploy as a CGI script (e.g. Apache mod_cgi):
    chmod +x echo_body.py
"""

import os
import sys


def read_stdin_body() -> bytes:
    try:
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    except ValueError:
        content_length = 0

    if content_length <= 0:
        return b''

    return sys.stdin.buffer.read(content_length)


def main():
    body = read_stdin_body()

    method = os.environ.get('REQUEST_METHOD', '')
    content_type = os.environ.get('CONTENT_TYPE', '')
    content_length = os.environ.get('CONTENT_LENGTH', '0')

    # CGI response: headers, blank line, then body
    sys.stdout.write("Content-Type: text/plain; charset=utf-8\r\n")
    sys.stdout.write("\r\n")

    sys.stdout.write(f"Method: {method}\n")
    sys.stdout.write(f"Content-Type: {content_type}\n")
    sys.stdout.write(f"Content-Length: {content_length}\n")
    sys.stdout.write(f"Bytes read: {len(body)}\n")
    sys.stdout.write("--- body ---\n")

    # write raw bytes as-is, don't assume it's valid utf-8
    sys.stdout.flush()
    sys.stdout.buffer.write(body)
    sys.stdout.buffer.write(b"\n")


if __name__ == '__main__':
    main()
