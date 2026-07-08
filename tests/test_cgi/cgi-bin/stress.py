#!/usr/bin/env python3

print("Content-Type: text/plain\r\n\r")

# Print 5 Megabytes of data (A string of 100 chars, printed 50,000 times)
chunk = "A" * 99 + "\n"
for _ in range(50000):
    print(chunk, end="")