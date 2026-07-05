#!/usr/bin/env python3
import os

# 1. CGI scripts MUST print their own HTTP headers first, ending with a blank line
print("Content-Type: text/plain\r\n\r")

# 2. Dump the environment to prove the server passed the data
print("=== CGI ENVIRONMENT VARIABLES ===")
for key, value in os.environ.items():
    if key in ["REQUEST_METHOD", "QUERY_STRING", "SERVER_PROTOCOL", "CONTENT_LENGTH"]:
        print(f"{key}: {value}")
print("=================================")