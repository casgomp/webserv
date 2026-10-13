#!/usr/bin/env python3

import os
import sys
import time

query = os.environ.get("QUERY_STRING", "")

if query == "mode=empty":
    sys.exit(0)

print("Content-Type: text/plain")
print()

if query == "mode=big":
    for i in range(500):
        print("line " + str(i) + ": this is a long line of filler text to make the output big")
elif query == "mode=fail":
    print("this output should be discarded")
    sys.exit(1)
elif query == "mode=sleep":
    time.sleep(5)
    print("slept 5 seconds")
elif query == "mode=loop":
    while True:
        pass
else:
    print("default response, query: " + query)