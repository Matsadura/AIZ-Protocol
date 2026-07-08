#!/usr/bin/env python3
import sys
import os

print("Content-Type: text/plain\r\n\r")
print("=== ECHOING POST DATA ===")

# Read the exact number of bytes specified by the server
content_length = int(os.environ.get("CONTENT_LENGTH", 0))
if content_length > 0:
    body = sys.stdin.read(content_length)
    print(body)
else:
    print("No body received.")