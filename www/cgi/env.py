import os, sys
sys.stdout.write('Content-Type: text/plain\r\n\r\n')
for k in ['REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_LENGTH', 'SERVER_PROTOCOL', 'HTTP_X_CUSTOM']:
    sys.stdout.write('%s=%s\n' % (k, os.environ.get(k, '<unset>')))
