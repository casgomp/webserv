#!/usr/bin/env python3

import sys
import os

content_length = os.environ.get("CONTENT_LENGTH", "")

if content_length == "":
	length = 0
else:
	length = int(content_length)

body = sys.stdin.buffer.read(length)

print("Content-Type: text/plain")
print()
print(body.decode())