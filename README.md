*This project has been created as part of the 42 curriculum by pecastro, ebara.*

# webserv

An HTTP/1.1 server written in C++98, built around a single `epoll` event loop.

## Description

**webserv** is an implementation of the 42 *Webserv* project: a web server that
is configured through an nginx-style configuration file and works with a real
browser, `curl`, `telnet` and `netcat`.

The goal is to understand how HTTP works underneath the tools used every day:
sockets, non-blocking I/O, request parsing, status codes, keep-alive connections,
file uploads and CGI.

What the server does:

- Serves a fully static website and lets clients upload and delete files
  (`GET`, `POST`, `DELETE`).
- Listens on several `address:port` pairs at once, and can serve different sites
  from the same port by `Host` name (`server_name`).
- Reads everything from a configuration file: ports, roots, allowed methods per
  location, redirections, directory listing, default index files, error pages,
  request body size limits and CGI interpreters.
- Runs CGI scripts selected by file extension (for example Python), un-chunking
  chunked request bodies before they reach the script.
- Handles keep-alive connections, pipelined requests and chunked request bodies.
- Answers with accurate status codes and default HTML error pages when none are
  configured.
- Uses **one** `epoll` instance for every socket and CGI pipe, all non-blocking.

## Instructions

### Requirements

- A C++ compiler (`c++`) supporting C++98, and `make`.
- Linux (`epoll`). No external libraries.
- Python 3 (only needed to run CGI scripts written in Python).

### Build

```bash
make          # builds ./webserv with -Wall -Wextra -Werror -std=c++98
make clean    # removes object files
make fclean   # removes object files and the binary
make re       # fclean + all
```

### Run

```bash
./webserv [configuration file]
```

When no file is given, the default configuration `config/default.conf` is used.
Every directory used as a `root` in the configuration must exist before the
server starts. Example with a configuration containing several servers:

```bash
./webserv config/multiple-servers.conf
curl -i http://localhost:<port>/     # <port> as defined by a listen directive in the file
```

The server is stopped with `Ctrl+C` (SIGINT); it closes its sockets and exits.
`Ctrl+Z` does not stop it: the process is only suspended (SIGTSTP) and keeps its
ports bound until it is resumed with `fg` or killed.

## Resources

### References

- [RFC 9110 – HTTP Semantics](https://www.rfc-editor.org/rfc/rfc9110)
- [RFC 9112 – HTTP/1.1](https://www.rfc-editor.org/rfc/rfc9112)
- [RFC 1945 – HTTP/1.0](https://www.rfc-editor.org/rfc/rfc1945) (the reference suggested by the subject)
- [RFC 3875 – The Common Gateway Interface (CGI)](https://www.rfc-editor.org/rfc/rfc3875)
- [RFC 3986 – URI generic syntax](https://www.rfc-editor.org/rfc/rfc3986) (percent-encoding, paths)
- [nginx documentation](https://nginx.org/en/docs/), used as the reference for headers and answers
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [`epoll(7)`](https://man7.org/linux/man-pages/man7/epoll.7.html) and the man pages of `socket`, `recv`, `send`, `fork`, `execve`, `pipe`
- [MDN – HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)

### How AI was used

An AI assistant was used for the following tasks. Everything it produced was
reviewed, tested and discussed by the team before it went into the repository.

- **Debugging:** analysing test failures, memory use with large CGI uploads, and
  event-loop behaviour with pipelined requests.
- **Tests:** drafting the end-to-end test suite from the edge cases of the unit
  tests, and suggesting manual commands to reproduce failures.
- **Documentation:** first drafts of `TESTING.md`, `WORKFLOW.md` and this README.
- **Frontend:** drafting and reviewing the HTML/CSS/JS of the static demo website.
- **Review:** explaining specific functions and suggesting small fixes, which were
  applied and tested by the team.

> **TODO (team):** confirm this list and add anything that is missing.

## Configuration

The format follows the nginx style: `http { server { location ... } }`, one
directive per line, each ending with `;`. Comments are not supported in
configuration files at the moment.

```
http {
    include mime.types;
    root www;
    server {
        listen localhost:8080;
        server_name example.com;
        client_max_body_size 1000000;
        error_page 404 /404.html;
        location / {
            allowed_methods GET;
            index index.html;
        }
        location /upload/ {
            allowed_methods GET POST DELETE;
            client_max_body_size 10;
        }
        location /old/ {
            allowed_methods GET;
            return 301 /new/;
        }
        location /cgi/ {
            allowed_methods GET POST;
            cgi_extension .py /usr/bin/python3;
        }
    }
}
```

| Directive | Where | Meaning |
|---|---|---|
| `include <file>` | `http` | MIME type table |
| `root <dir>` | `http`, `server`, `location` | directory the URL path is resolved against; must exist |
| `client_max_body_size <bytes>` | `http`, `server`, `location` | largest accepted request body; larger bodies get `413` (default 1 MiB) |
| `index <file> [file...]` | `http`, `server`, `location` | files tried, in order, when a directory is requested; `index.html` is the fallback |
| `autoindex on\|off` | `http`, `server`, `location` | directory listing when there is no index file (default `off`) |
| `listen <port>` / `<address>:<port>` / `*:<port>` | `server` | interface and port; several `listen` lines allowed |
| `server_name <name> [name...]` | `server` | names matched against the `Host` header when several servers share a port |
| `error_page <code> [code...] <path>` | `server` | custom error page |
| `allowed_methods <GET\|POST\|DELETE>...` | `location` | accepted methods; others get `405` |
| `return <3xx> <url>` | `location` | HTTP redirection |
| `cgi_extension <.ext> <interpreter>` | `location` | run files with this extension through the interpreter |

`location` paths are matched by longest prefix on path-segment boundaries
(`/fruits` matches `/fruits/red` but not `/fruitsaaaa`).

Ready-made examples are in `config/`.

## How it behaves

### Request handling

1. One `epoll` loop accepts clients and drives all sockets and CGI pipes.
2. The parser reads the request incrementally (it can arrive in pieces) and
   reports *incomplete*, *complete* or *error*.
3. The `Host` header selects the server block; the longest matching `location`
   selects the rules.
4. Validation decides the outcome (method, redirection, file, directory, CGI,
   upload, delete) and the response is built and sent without blocking.

### Status codes

| Code | When |
|---|---|
| 200 / 201 / 204 | success / file uploaded / file deleted |
| 301 / 3xx | directory requested without a trailing `/`, or a configured `return` |
| 400 | malformed request (request line, headers, `Host`, `Content-Length`, chunks) |
| 403 / 404 | forbidden (permissions, directory without index or autoindex) / not found |
| 405 | method not allowed for the location, or unknown method |
| 413 | request body larger than `client_max_body_size` |
| 414 | request line too long |
| 431 | header section too large |
| 500 / 504 | server or CGI failure / CGI timeout |
| 501 | unsupported `Transfer-Encoding` |
| 505 | HTTP version other than 1.1 |

Connections are kept alive after fully-read requests. They are closed after
requests that could not be parsed or were refused without reading their body
(`400`, `414`, `431`, `501`, `505`) and when the client sends
`Connection: close`.

### Limits and timeouts

| Limit | Value |
|---|---|
| Request line | 8000 bytes (`414` above; the minimum RFC 9112 recommends supporting) |
| Header section | 16 KiB (`431` above; own choice, the RFCs set no limit) |
| Idle client | 60 s, then the connection is closed silently |
| CGI script | 15 s without activity, then `504` |

### CGI

A file whose extension is listed in `cgi_extension` is executed with the given
interpreter (`fork` + `execve`, output through pipes). The environment contains
`REQUEST_METHOD`, `SERVER_PROTOCOL`, `PATH_INFO`, `QUERY_STRING` (GET),
`CONTENT_LENGTH` and `CONTENT_TYPE` (POST) and every request header as
`HTTP_*`. Chunked request bodies are un-chunked before they are written to the
script's stdin, and the end of the script's output marks the end of the response
when it does not send a length. A script that fails gives `500`; one that hangs
is killed and gives `504`.

## Known limitations

- Only HTTP/1.1 is accepted.
- Request bodies are held in memory, so very large uploads (and many of them at
  once) use a lot of RAM; there is no streaming yet.
- `POST` to a target that does not exist and has no extension, or an extension
  with no known MIME type, is accepted (`200`) and discarded. `POST` to an
  existing file with such an extension gets `415`.
- No cookies/sessions and only the CGI types the configuration lists.

## Repository layout

```
Makefile        build rules (NAME, all, clean, fclean, re, test)
src/            server sources
include/        headers
config/         example configuration files
content/        default root used when none is configured
tests/          unit tests and the end-to-end suite
testers/        files used to run the tester provided with the subject (see TESTING.md)
.github/        CI workflow (builds and runs `make test` on pull requests)
TESTING.md      testing guide
WORKFLOW.md     team workflow
```

## Testing

The project has three layers of tests:

- **Unit tests** for the request parser, the server logic (routing, paths, MIME
  types, listening sockets) and the response builder.
- **End-to-end tests** that start the real `./webserv` and talk to it over
  sockets, organised by category (response format, request line, headers,
  body, connection handling, methods, static files, routing, redirects,
  CGI, configuration).
- The tester provided with the subject, run by hand.

```bash
make test     # builds the server and runs the unit and end-to-end suites
```

The exit code is `0` only if every test passes. See [`TESTING.md`](TESTING.md)
for the full procedure (manual checks, the provided tester, memory
measurements); [`WORKFLOW.md`](WORKFLOW.md) describes the branches, pull
requests and CI.