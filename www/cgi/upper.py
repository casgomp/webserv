import sys
data = sys.stdin.buffer.read()
sys.stdout.buffer.write(b'Content-Type: text/plain\r\n\r\n' + data.upper())
