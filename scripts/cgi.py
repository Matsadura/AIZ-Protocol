#!/usr/bin/env python3
import os
import sys

# Path to the WebP image on the server
IMAGE_PATH = "./streamme.mp4"

# Write headers (using standard print for text)
print("Content-Type: video/mp4")
print(f"Content-Length: {os.path.getsize(IMAGE_PATH)}")
print()

# Flush the text buffer before writing raw binary data
sys.stdout.flush()

try:
    with open(IMAGE_PATH, "rb") as image_file:
        # Read the raw binary data
        binary_data = image_file.read()
        
        # Write directly to the standard output buffer
        # This prevents Python from attempting to encode the binary data as text
        sys.stdout.buffer.write(binary_data)
        
        # Flush the buffer to ensure all bytes are sent
        sys.stdout.buffer.flush()
        
except FileNotFoundError:
    # Optional: Handle the error by outputting a 404 response
    print("Status: 404 Not Found")
    print("Content-Type: text/plain")
    print()
    print("Image not found.")
