#!/usr/bin/env python3
"""
echo_body.py minimal CGI script to check what body the server hands you
"""

import os
import sys


def read_stdin_body() -> bytes:
    body = b''
    while True:
        data = sys.stdin.buffer.read() 
        body += data
        if len(data) == 0:
            return body

def main():
    body = read_stdin_body()

    method = os.environ.get('REQUEST_METHOD', '')
    content_type = os.environ.get('CONTENT_TYPE', '')
    content_length = os.environ.get('CONTENT_LENGTH', '0')

    sys.stdout.write("Content-Type: text/plain; charset=utf-8\r\n")
    sys.stdout.write("\r\n")

    sys.stdout.flush()
    sys.stdout.buffer.write(body)


if __name__ == '__main__':
    main()
