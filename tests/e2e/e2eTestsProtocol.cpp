# include <unistd.h>

# include "e2eTests.hpp"

using namespace e2e;

/*
** Categories 1-5: the HTTP protocol itself, same order as HttpRequestParserTests
** (general -> request line -> headers/framing -> body -> consumption/pipelining),
** preceded by the response format (HttpResponseTests).
*/

/* ------------------------- 1. response format ------------------------- */

static bool	isHtml(const Reply &r)
{
	return (startsWith(r.header("content-type"), "text/html"));
}

void	e2eTests::test_responseFormat()
{
	section("1. RESPONSE FORMAT  (status line, headers, error pages)");

	Reply	ok = exchange(get("/"));
	check(ok.status == 200 && ok.version == "HTTP/1.1" && ok.reason == "OK",
		"[R] 200 status line is 'HTTP/1.1 200 OK'");
	check(ok.body == readFile("www/index.html"), "[R] GET file: body is the file, byte for byte");
	check(ok.framed && ok.header("content-length") == toStr(ok.body.size()),
		"[R] GET file: Content-Length matches the body");
	check(startsWith(ok.header("content-type"), "text/html"), "[R] GET .html: Content-Type text/html");
	check(ok.header("connection") == "keep-alive", "[R] keep-alive request: 'Connection: keep-alive'");

	Reply	missing = exchange(get("/nope"));
	check(missing.status == 404 && missing.reason == "Not Found", "[R] 404 status line is 'HTTP/1.1 404 Not Found'");
	check(contains(missing.body, "<h1>404 Not Found</h1>"), "[R] 404 has an HTML error page '<h1>404 Not Found</h1>'");
	check(contains(missing.body, "HTTP Amigos/1.0.0"), "[R] error page carries the server footer");
	check(isHtml(missing), "[R] error page: Content-Type text/html");
	check(missing.framed && missing.header("content-length") == toStr(missing.body.size()),
		"[R] error page: Content-Length matches the body");

	Reply	r405 = exchange(raw("POST", "/", "HTTP/1.1", "Host: localhost\r\nContent-Length: 0\r\n"));
	check(r405.status == 405 && r405.reason == "Method Not Allowed", "[R] 405 reason phrase is 'Method Not Allowed'");
	check(contains(r405.body, "<h1>405 Method Not Allowed</h1>") && isHtml(r405), "[R] 405 HTML error page");

	Reply	r403 = exchange(get("/noauto/"));
	check(r403.status == 403 && r403.reason == "Forbidden", "[R] 403 reason phrase is 'Forbidden'");
	check(contains(r403.body, "<h1>403 Forbidden</h1>") && isHtml(r403), "[R] 403 HTML error page");

	Reply	r400 = exchange("GARBAGE\r\n\r\n", false, true);
	check(r400.status == 400 && r400.reason == "Bad Request", "[R] 400 reason phrase is 'Bad Request'");
	check(contains(r400.body, "<h1>400 Bad Request</h1>") && isHtml(r400), "[R] 400 HTML error page");
	check(r400.header("connection") == "close" && r400.closed, "[R] 400: 'Connection: close' header and socket really closed");

	Reply	r413 = exchange(post("/tiny/x.txt", std::string(11, 'x')));
	check(r413.status == 413 && r413.reason == "Content Too Large", "[R] 413 reason phrase is 'Content Too Large'");
	check(contains(r413.body, "<h1>413 Content Too Large</h1>") && isHtml(r413), "[R] 413 HTML error page");

	Reply	r414 = exchange(get("/" + std::string(9000, 'a')));
	check(r414.status == 414 && r414.reason == "URI Too Long", "[R] 414 reason phrase is 'URI Too Long'");
}

/* --------------------------- 2. request line --------------------------- */

void	e2eTests::test_requestLine()
{
	section("2. REQUEST LINE  (method, spacing, line ending, target, query, version, length)");

	// general
	expectOpen(get("/"), 200, "[P] valid request -> 200");

	// method / spacing / line ending
	expectClosed("G@T / HTTP/1.1\r\nHost: localhost\r\n\r\n", 400, "[P] malformed method 'G@T' -> 400");
	expectClosed("GET  / HTTP/1.1\r\nHost: localhost\r\n\r\n", 400, "[P] two spaces in the request line -> 400");
	expectClosed("GET / HTTP/1.1\nHost: localhost\r\n\r\n", 400, "[P] request line ending in bare LF -> 400");
	expectClosed("GARBAGE\r\n\r\n", 400, "[T] garbage instead of a request line -> 400");
	expectOpen(raw("PUT", "/", "HTTP/1.1", "Host: localhost\r\n"), 405, "[P] unsupported method PUT parses fine, rejected as 405");
	expectOpen(raw("PIZZA", "/", "HTTP/1.1", "Host: localhost\r\n"), 405, "[S] unknown method PIZZA -> 405");
	expectOpen(raw("FOO", "/", "HTTP/1.1", "Host: localhost\r\n"), 405, "[T] unknown method FOO -> 405");

	// origin-form target
	expectOpen(get("/index.html"), 200, "[P] origin-form target -> 200");
	expectOpen(get("/hello%20world"), 404, "[P] valid percent-encoded target is accepted (just not found)");
	expectClosed(get("/hello%GGworld"), 400, "[P] invalid percent-encoding '%GG' -> 400");
	expectClosed(get("/hello%"), 400, "[P] incomplete percent-encoding '%' -> 400");
	expectClosed("GET /hello world HTTP/1.1\r\nHost: localhost\r\n\r\n", 400, "[P] space inside the target -> 400");
	expectClosed("GET /hello\tworld HTTP/1.1\r\nHost: localhost\r\n\r\n", 400, "[P] tab inside the target -> 400");
	expectClosed(get("/hello#frag"), 400, "[P] fragment in the target -> 400");

	// query string
	expectOpen(get("/search?q=hello"), 404, "[P] query string is accepted");
	expectOpen(get("/search?a=1?b=2"), 404, "[P] second '?' inside the query is accepted");
	expectOpen(get("/search?q=hello%20world"), 404, "[P] percent-encoding inside the query is accepted");
	expectClosed(get("/search?q={bad}"), 400, "[P] invalid characters in the query -> 400");
	expectOpen(get("/index.html?x=1"), 200, "[P] query is stripped from the path that gets served");

	// HTTP version
	expectOpen(get("/index.html"), 200, "[P] HTTP/1.1 -> 200");
	expectClosed(raw("GET", "/index.html", "HTTP/1.0", "Host: localhost\r\n"), 505, "[P] HTTP/1.0 -> 505");
	expectClosed(raw("GET", "/", "HTTP/2.0", "Host: localhost\r\n"), 505, "[P] HTTP/2.0 -> 505");
	expectClosed(raw("GET", "/index.html", "http/1.1", "Host: localhost\r\n"), 400, "[P] lower-case 'http/1.1' -> 400");

	// request line length (limit: 8000 bytes without the CRLF)
	expectOpen("GET /" + std::string(7986, 'a') + " HTTP/1.1\r\nHost: localhost\r\n\r\n", 404,
		"[P] request line of exactly 8000 bytes is accepted");
	expectClosed("GET /" + std::string(8001, 'a') + " HTTP/1.1\r\nHost: localhost\r\n\r\n", 414,
		"[P] request line over 8000 bytes -> 414");
	expectClosed("GET /" + std::string(7988, 'a') + " HTTP/1.1", 414,
		"[P] over-long request line is refused even before its CRLF arrives -> 414");

	// pieces of the request line may arrive separately
	dribble("GET / HTT", "P/1.1\r\nHost: localhost\r\n\r\n", 200, "[P] request line split across two packets");
	dribble("GET / HTTP/1.1", "\r\nHost: localhost\r\n\r\n", 200, "[P] incomplete request line (no CRLF yet)");
}

/* ------------------------ 3. headers and framing ------------------------ */

void	e2eTests::test_headersAndFraming()
{
	section("3. HEADERS AND FRAMING  (size limit, Host, Content-Length, Transfer-Encoding)");

	// header block size
	std::string	many;
	for (int i = 0; i < 1000; ++i)
		many += "X-Test: abcdefghij\r\n";
	expectClosed("GET / HTTP/1.1\r\nHost: localhost\r\n" + many + "\r\n", 431,
		"[P] header block over the limit -> 431");
	expectClosed("GET / HTTP/1.1\r\nHost: localhost\r\n" + many, 431,
		"[P] over-sized header block is refused even without the final blank line -> 431");
	expectOpen(get("/", "X-Test: abcdefghij\r\n"), 200, "[P] small header block below the limit -> 200");

	// Host
	expectOpen(get("/index.html"), 200, "[P] single valid Host -> 200");
	expectClosed(raw("GET", "/index.html", "HTTP/1.1"), 400, "[P] missing Host -> 400");
	expectClosed(raw("GET", "/index.html", "HTTP/1.1", "Host: \r\n"), 400, "[P] empty Host -> 400");
	expectClosed(raw("GET", "/index.html", "HTTP/1.1", "Host: localhost\r\nHost: example.com\r\n"), 400,
		"[P] duplicate Host -> 400");
	expectOpen(raw("GET", "/index.html", "HTTP/1.1", "HOST: localhost\r\n"), 200,
		"[P] header names are case-insensitive (HOST)");
	expectClosed(raw("GET", "/index.html", "HTTP/1.1", "Host: localhost\r\nInvalidHeader\r\n"), 400,
		"[P] header line without ':' -> 400");

	// Content-Length
	expectOpen(raw("GET", "/index.html", "HTTP/1.1", "Host: localhost\r\nContent-Length: 5\r\n") + "hello", 200,
		"[P] Content-Length body on GET is read and ignored");
	expectClosed(raw("GET", "/index.html", "HTTP/1.1", "Host: localhost\r\nContent-Length: abc\r\n") + "hello", 400,
		"[P] non-numeric Content-Length -> 400");
	expectClosed(raw("GET", "/index.html", "HTTP/1.1", "Host: localhost\r\nContent-Length:\r\n") + "hello", 400,
		"[P] empty Content-Length -> 400");
	expectOpen(raw("GET", "/index.html", "HTTP/1.1", "Host: localhost\r\nContent-Length: 5\r\nContent-Length: 5\r\n") + "hello", 200,
		"[P] duplicate identical Content-Length is accepted");
	expectClosed(raw("GET", "/index.html", "HTTP/1.1", "Host: localhost\r\nContent-Length: 5\r\nContent-Length: 6\r\n") + "hello", 400,
		"[P] conflicting Content-Length values -> 400");
	expectClosed(raw("GET", "/index.html", "HTTP/1.1",
		"Host: localhost\r\nContent-Length: 999999999999999999999999999999999999\r\n") + "hello", 400,
		"[P] overflowing Content-Length -> 400");
	expectOpen(raw("GET", "/index.html", "HTTP/1.1", "Host: localhost\r\nContent-Length: 0\r\n"), 200,
		"[P] Content-Length: 0 -> empty body");

	// Transfer-Encoding
	expectClosed(raw("GET", "/index.html", "HTTP/1.1",
		"Host: localhost\r\nContent-Length: 5\r\ntransfer-encoding: chunked\r\n") + "hello\r\n", 400,
		"[P] Content-Length together with Transfer-Encoding -> 400");
	expectClosed(raw("POST", "/upload/te.txt", "HTTP/1.1", "Host: localhost\r\nTransfer-Encoding: gzip\r\n"), 501,
		"[P] unsupported Transfer-Encoding: gzip -> 501");
	expectClosed(raw("POST", "/upload/te.txt", "HTTP/1.1",
		"Host: localhost\r\nTransfer-Encoding: chunked\r\nTransfer-Encoding: chunked\r\n") + "0\r\n\r\n", 400,
		"[P] duplicate Transfer-Encoding -> 400");
	expectOpen(raw("GET", "/index.html", "HTTP/1.1", "Host: localhost\r\ntransfer-encoding: chunked\r\n") + "0\r\n\r\n", 200,
		"[P] Transfer-Encoding name is case-insensitive");

	// header block may arrive in pieces
	dribble("GET / HTTP/1.1\r\nHost: loc", "alhost\r\n\r\n", 200, "[P] header split across two packets");
	dribble("GET / HTTP/1.1\r\nHost: localhost\r\n", "\r\n", 200, "[P] incomplete headers (no blank line yet)");
}

/* --------------------------------- 4. body --------------------------------- */

void	e2eTests::test_body()
{
	section("4. BODY  (Content-Length, chunked, size limits, partial arrival)");

	// -- Content-Length bodies end up in the file, exactly --
	removeTree("www/upload");
	mkdirs("www/upload/ro");
	expect(post("/upload/cl.txt", "hello"), 201, "[R] POST with Content-Length -> 201");
	check(readFile("www/upload/cl.txt") == "hello", "[R] POST body is written to the target file, exactly");

	std::string	binary;
	for (int i = 0; i < 256; i++)
		binary += static_cast<char>(i);			// includes \0 and \r\n
	expect(post("/upload/bin.txt", binary, "application/octet-stream"), 201, "[T] POST binary body (all 256 byte values) -> 201");
	check(readFile("www/upload/bin.txt") == binary, "[T] binary body stored without alteration");

	std::string	whole = post("/upload/part.txt", "hello");
	dribble(whole.substr(0, whole.size() - 3), "llo", 201, "[P] Content-Length body arriving in two pieces");
	check(readFile("www/upload/part.txt") == "hello", "[P] ...and stored whole");

	// -- chunked: valid --
	expect(postChunked("/upload/ch1.txt", chunk("hello") + chunk(" world") + "0\r\n\r\n"), 201,
		"[P] chunked request with two chunks -> 201");
	check(readFile("www/upload/ch1.txt") == "hello world", "[P] chunks are joined into one body");

	expect(postChunked("/upload/ch2.txt", "A\r\n0123456789\r\n0\r\n\r\n"), 201, "[P] upper-case hex chunk size 'A' -> 201");
	check(readFile("www/upload/ch2.txt") == "0123456789", "[P] 'A' chunk body is correct");

	expect(postChunked("/upload/ch3.txt", "5;foo=bar\r\nhello\r\n0\r\n\r\n"), 201, "[P] chunk extension '5;foo=bar' -> 201");
	check(readFile("www/upload/ch3.txt") == "hello", "[P] chunk extension is ignored, body kept");

	expect(postChunked("/upload/ch4.txt", "5\r\nhello\r\n0\r\nX-Test: value\r\n\r\n"), 201, "[P] chunked request with one trailer -> 201");
	check(readFile("www/upload/ch4.txt") == "hello", "[P] trailer is not part of the body");
	expect(postChunked("/upload/ch5.txt", "5\r\nhello\r\n0\r\nX-One: value\r\nX-Two: another\r\n\r\n"), 201,
		"[P] chunked request with several trailers -> 201");

	// -- chunked: invalid --
	expectClosed(postChunked("/upload/bad.txt", "5\r\nhelloXX0\r\n\r\n"), 400, "[P] malformed chunk terminator -> 400");
	expectClosed(postChunked("/upload/bad.txt", "ZZ\r\nhello\r\n0\r\n\r\n"), 400, "[P] invalid chunk size 'ZZ' -> 400");
	expectClosed(postChunked("/upload/bad.txt", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\r\n"), 400, "[P] overflowing chunk size -> 400");
	expectClosed(postChunked("/upload/bad.txt", "ZZ;foo=bar\r\nhello\r\n0\r\n\r\n"), 400, "[P] invalid size with extension -> 400");
	expectClosed(postChunked("/upload/bad.txt", "5\r\nhello\r\n0\r\nInvalidTrailer\r\n\r\n"), 400, "[P] invalid trailer (no ':') -> 400");
	check(!fileExists("www/upload/bad.txt"), "[T] a rejected request creates no file");

	// -- chunked: arrives in pieces (INCOMPLETE cases) --
	dribble(postChunked("/upload/p1.txt", "5\r\nhello\r\n6\r\n wor"), "ld\r\n0\r\n\r\n", 201, "[P] chunked body cut in the middle of a chunk");
	check(readFile("www/upload/p1.txt") == "hello world", "[P] ...and joined correctly");
	dribble(postChunked("/upload/p2.txt", "5"), "\r\nhello\r\n0\r\n\r\n", 201, "[P] chunk size line not finished yet");
	dribble(postChunked("/upload/p3.txt", "5\r\nhello\r\n0\r\n"), "\r\n", 201, "[P] final chunk without its blank line yet");
	dribble(postChunked("/upload/p4.txt", "5\r\nhello\r\n0\r\nX-Test: val"), "ue\r\n\r\n", 201, "[P] trailer cut in the middle");

	// -- size limits (server: 1000, /tiny/: 10) --
	expect(post("/upload/lim1.txt", std::string(1000, 'a')), 201, "[P] Content-Length exactly at the server limit (1000) is allowed");
	expect(post("/upload/lim2.txt", std::string(1001, 'a')), 413, "[P] Content-Length over the server limit -> 413");
	expect(post("/tiny/x.txt", std::string(10, 'a')), 201, "[P] Content-Length exactly at the location limit (10) is allowed");
	expect(post("/tiny/x.txt", std::string(11, 'a')), 413, "[S] Content-Length over the location limit (10) -> 413");
	expectNot(postChunked("/tiny/y.txt", chunk("ab") + chunk("cde") + "0\r\n\r\n"), 413, "[P] chunked body exactly at the limit (5+5=10) is allowed");
	expect(postChunked("/tiny/y.txt", chunk("abcdef") + chunk("ghijkl") + "0\r\n\r\n"), 413, "[P] chunked body over the limit -> 413");
	expect(postChunked("/upload/lim3.txt", chunk(std::string(600, 'a')) + chunk(std::string(600, 'b')) + "0\r\n\r\n"), 413,
		"[P] chunked body over the (inherited) server limit -> 413");
}

/* -------------------- 5. connection handling / pipelining -------------------- */

void	e2eTests::test_connection()
{
	section("5. CONNECTION  (keep-alive, close rules, pipelining, consumed bytes)");

	// close rules: unparsable / refused unread -> close; fully read -> keep-alive
	expectOpen(get("/"), 200, "[T] 200 keeps the connection");
	expectOpen(get("/nope"), 404, "[T] 404 keeps the connection");
	expectOpen(raw("POST", "/", "HTTP/1.1", "Host: localhost\r\nContent-Length: 0\r\n"), 405, "[T] 405 keeps the connection");
	expectOpen(get("/noauto/"), 403, "[T] 403 keeps the connection");
	expectClosed("GARBAGE\r\n\r\n", 400, "[T] 400 closes the connection");
	expectClosed(get("/" + std::string(10000, 'a')), 414, "[T] 414 closes the connection");
	expectClosed(raw("GET", "/", "HTTP/1.0", "Host: localhost\r\n"), 505, "[T] 505 closes the connection");

	{
		Reply	r = exchange(get("/", "Connection: close\r\n"), false, true);
		check(r.status == 200 && r.closed && r.header("connection") == "close",
			"[T] client sends 'Connection: close' -> answered, then closed");
	}

	// several requests on ONE connection
	{
		Conn	c;
		Reply	a, b, d;

		c.send(get("/index.html"));
		a = c.read();
		c.send(get("/nope"));
		b = c.read();
		c.send(get("/second.html"));
		d = c.read();
		check(a.status == 200 && b.status == 404 && d.status == 200 && d.body == "second page\n",
			"[T] three sequential requests on one connection");
	}

	// pipelining: all requests in one write, answers in order
	{
		Conn	c;
		Reply	a, b;

		c.send(get("/index.html") + get("/second.html"));
		a = c.read();
		b = c.read();
		check(a.status == 200 && b.status == 200 && a.body == readFile("www/index.html") && b.body == "second page\n",
			"[P] two pipelined GETs -> two answers, in order");
	}
	{
		Conn	c;
		Reply	a, b, d;

		c.send(get("/index.html") + get("/nope") + get("/second.html"));
		a = c.read();
		b = c.read();
		d = c.read();
		check(a.status == 200 && b.status == 404 && d.status == 200, "[P] three pipelined requests (200, 404, 200)");
	}
	{
		Conn	c;
		Reply	a, b;

		c.send(post("/upload/pl1.txt", "hello") + get("/second.html"));
		a = c.read();
		b = c.read();
		check(a.status == 201 && b.status == 200 && b.body == "second page\n",
			"[P] Content-Length POST consumes its body, not the pipelined request");
		check(readFile("www/upload/pl1.txt") == "hello", "[P] ...and the body is exactly 'hello'");
	}
	{
		Conn	c;
		Reply	a, b;

		c.send(postChunked("/upload/pl2.txt", chunk("hello") + "0\r\n\r\n") + get("/second.html"));
		a = c.read();
		b = c.read();
		check(a.status == 201 && b.status == 200 && b.body == "second page\n",
			"[P] chunked POST consumes its body, not the pipelined request");
	}
	{
		Conn	c;
		Reply	a, b;

		c.send(postChunked("/upload/pl3.txt", chunk("hello") + "0\r\nX-Test: value\r\n\r\n") + get("/second.html"));
		a = c.read();
		b = c.read();
		check(a.status == 201 && b.status == 200 && b.body == "second page\n",
			"[P] chunked POST with trailer consumes the trailer too");
	}
	{
		Conn	c;
		Reply	a, b;

		c.send(raw("GET", "/index.html", "HTTP/1.1", "Host: localhost\r\nContent-Length: 5\r\n") + "hello" + get("/second.html"));
		a = c.read();
		b = c.read();
		check(a.status == 200 && b.status == 200 && b.body == "second page\n",
			"[P] GET carrying a Content-Length body: body consumed, next request intact");
	}
	{
		Conn	c;
		Reply	a, b;

		c.send(get("/index.html") + "GARBAGE\r\n\r\n");
		a = c.read();
		b = c.read();
		check(a.status == 200 && b.status == 400 && c.peerClosed(1000),
			"[P] pipelined: good request answered, then the bad one -> 400 and close");
	}

	// the same request, one byte at a time
	{
		Conn		c;
		Reply		r;
		std::string	req = get("/index.html");

		for (size_t i = 0; i < req.size(); i++)
		{
			c.send(req.substr(i, 1));
			usleep(2000);
		}
		r = c.read();
		check(r.status == 200 && r.body == readFile("www/index.html"), "[T] request delivered one byte at a time");
	}
}
