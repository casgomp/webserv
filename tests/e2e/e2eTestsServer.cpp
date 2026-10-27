# include <cstdlib>
# include <iostream>
# include <sys/stat.h>
# include <unistd.h>

# include "e2eTests.hpp"

using namespace e2e;

/*
** Categories 6-10: what the server does with a well-formed request.
** Order: status codes by method -> paths/static files -> routing -> redirect/autoindex/index -> CGI.
*/

/* ------------------------------ 6. methods ------------------------------ */

void	e2eTests::test_methods()
{
	section("6. METHODS  (status codes per method)");

	/* ---- GET ---- */
	expectOpen(get("/"), 200, "GET / (index.html fallback) -> 200");
	expectOpen(get("/a.txt"), 200, "GET existing file -> 200");
	expectOpen(get("/nope"), 404, "GET missing file -> 404");
	expectOpen(get("/noauto/"), 403, "GET directory, autoindex off, no index -> 403");
	expectOpen(get("/fallback/"), 200, "GET directory with index.html and no 'index' directive -> 200");
	check(exchange(get("/fallback/")).body == "fallback index\n", "GET directory serves its index.html");
	expectOpen(get("/dir"), 301, "GET directory without trailing slash -> 301");
	if (!_isRoot)
	{
		writeFile("www/secret.txt", "secret\n");
		chmod("www/secret.txt", 0);
		expectOpen(get("/secret.txt"), 403, "GET file without read permission -> 403");
		chmod("www/secret.txt", 0644);
	}
	expect(raw("GET", "/upload/", "HTTP/1.1", "Host: localhost\r\n"), 403, "GET /upload/ (directory, no autoindex) -> 403");

	/* ---- POST ---- */
	expectOpen(raw("POST", "/", "HTTP/1.1", "Host: localhost\r\nContent-Length: 0\r\n"), 405, "POST where only GET is allowed -> 405");
	expectOpen(post("/upload/new.txt", "data"), 201, "POST to an allowed location -> 201");
	check(readFile("www/upload/new.txt") == "data", "POST creates the file with the body");
	expect(post("/upload/ro/x.txt", "data"), 405, "POST into the more specific, GET-only location -> 405");
	check(!fileExists("www/upload/ro/x.txt"), "a 405 POST creates nothing");
	expect(post("/upload/nodir/x.txt", "data"), 404, "POST into a directory that does not exist -> 404");
	if (!_isRoot)
	{
		mkdirs("www/all/readonly");
		chmod("www/all/readonly", 0555);
		expect(post("/all/readonly/x.txt", "data"), 403, "POST into a directory that is not writable -> 403");
		chmod("www/all/readonly", 0755);
	}
	expect(post("/tiny/z.txt", std::string(11, 'a')), 413, "POST over the location's client_max_body_size -> 413");
	// requestValidation: a target whose extension has no MIME type counts as "application/octet-stream"
	// (the school tester's POST /post_body): new file -> 200 and the body is dropped, existing file -> 415
	expect(post("/upload/x.unknownext", "data"), 200, "POST to a new file with an unknown extension -> 200 (accepted, discarded)");
	check(!fileExists("www/upload/x.unknownext"), "...and nothing is stored for it");
	writeFile("www/upload/exists.unknownext", "old");
	expect(post("/upload/exists.unknownext", "data"), 415, "POST to an existing file with an unknown extension -> 415");
	expectNot(post("/upload/", "data"), 0, "POST onto a directory path gets an answer (no crash, no hang)");

	/* ---- DELETE ---- */
	writeFile("www/all/del1.txt", "bye\n");
	expectOpen(del("/all/del1.txt"), 204, "DELETE existing file -> 204");
	check(!fileExists("www/all/del1.txt"), "DELETE removes the file");
	{
		Reply	r = exchange(del("/all/del1.txt"));
		check(r.status == 403 || r.status == 404, "DELETE a file that is already gone -> 403/404");
		if (r.status != 403 && r.status != 404)
			std::cout << "      got: " << r.statusLine() << std::endl;
	}
	writeFile("www/del2.txt", "keep me\n");
	expect(del("/del2.txt"), 405, "DELETE where only GET is allowed -> 405");
	check(fileExists("www/del2.txt"), "a 405 DELETE leaves the file alone");
	mkdirs("www/all/subdir");
	expect(del("/all/subdir/"), 403, "DELETE a directory (with trailing '/') -> 403");
	expect(del("/all/subdir"), 301, "DELETE a directory without '/' -> 301 (the directory redirect runs before the method is looked at)");
	check(isDir("www/all/subdir"), "...and the directory is still there");
	if (!_isRoot)
	{
		mkdirs("www/all/locked");
		writeFile("www/all/locked/f.txt", "x");
		chmod("www/all/locked", 0555);
		expect(del("/all/locked/f.txt"), 403, "DELETE in a directory that is not writable -> 403");
		check(fileExists("www/all/locked/f.txt"), "...and the file is still there");
		chmod("www/all/locked", 0755);
	}

	/* ---- HEAD / unknown methods ---- */
	{
		Reply	h = exchange(head("/index.html"), true);
		check(h.status == 200 || h.status == 405, "HEAD gets a definite answer (200, or 405 if not supported)");
		check(h.body.empty(), "HEAD never carries a body");
	}
	expectOpen(raw("PUT", "/upload/p.txt", "HTTP/1.1", "Host: localhost\r\nContent-Length: 0\r\n"), 405, "PUT -> 405");
	check(!fileExists("www/upload/p.txt"), "PUT creates nothing");
	expectOpen(raw("PATCH", "/", "HTTP/1.1", "Host: localhost\r\n"), 405, "PATCH -> 405");
	expectOpen(raw("OPTIONS", "/", "HTTP/1.1", "Host: localhost\r\n"), 405, "OPTIONS -> 405");
	expectOpen(raw("TRACE", "/", "HTTP/1.1", "Host: localhost\r\n"), 405, "TRACE -> 405");
}

/* ------------------------ 7. paths and static files ------------------------ */

void	e2eTests::test_pathsAndStaticFiles()
{
	section("7. PATHS AND STATIC FILES  (normalisation, traversal, MIME types, integrity)");

	// normalizePath / joinedPath
	expectOpen(get("/./index.html"), 200, "[S] '/./' segment is dropped");
	expectOpen(get("/dir/../index.html"), 200, "[S] '..' resolves inside the root");
	expectOpen(get("//index.html"), 200, "[S] double slash is collapsed");
	expectOpen(get("/dir//index.html"), 200, "[S] double slash inside the path is collapsed");
	expectOpen(get("/dir/./../a.txt"), 200, "[S] mixed '.' and '..'");
	expectOpen(get("/a.txt/"), 404, "[T] trailing slash on a file -> 404, not the file");

	// traversal must never leave the root
	{
		const char	*attacks[] = {"/../../etc/passwd", "/../etc/passwd", "/dir/../../../etc/passwd",
			"/%2e%2e/%2e%2e/etc/passwd", "/..%2f..%2fetc/passwd", "/....//....//etc/passwd"};

		for (size_t i = 0; i < sizeof(attacks) / sizeof(attacks[0]); i++)
		{
			Reply	r = exchange(get(attacks[i]));
			check(r.status != 200 && r.status != 0 && !contains(r.body, "root:"),
				std::string("[S] traversal '") + attacks[i] + "' does not leak files");
			if (r.status == 200 || r.status == 0)
				std::cout << "      got: " << r.statusLine() << std::endl;
		}
	}

	// percent-decoding of the path
	{
		Reply	r = exchange(get("/hello%20world.txt"));
		check(r.status == 200 && r.body == "name with a space\n", "[P] '%20' in the path is decoded to find 'hello world.txt'");
		if (r.status != 200)
			std::cout << "      got: " << r.statusLine() << std::endl;
	}
	expectOpen(get("/%61.txt"), 200, "[P] '%61' decodes to 'a'");

	// content types come from the extension
	{
		struct { const char *path; const char *type; }	cases[] = {
			{"/index.html", "text/html"}, {"/a.txt", "text/plain"}, {"/a.json", "application/json"},
			{"/a.css", "text/css"}, {"/a.mp4", "video/mp4"}};

		for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
		{
			Reply	r = exchange(get(cases[i].path));
			check(startsWith(r.header("content-type"), cases[i].type),
				std::string("[S] Content-Type for ") + cases[i].path + " is " + cases[i].type);
			if (!startsWith(r.header("content-type"), cases[i].type))
				std::cout << "      got Content-Type: " << r.header("content-type") << std::endl;
		}
	}

	// bytes arrive intact
	{
		Reply	r = exchange(get("/binary.bin"));
		check(r.status == 200 && r.body == readFile("www/binary.bin"), "[T] binary file (all byte values, 1 KiB) served unchanged");
		r = exchange(get("/big.txt"));
		check(r.status == 200 && r.complete && r.body == readFile("www/big.txt"), "[T] 1 MB file served complete and unchanged");
		check(r.header("content-length") == "1000000", "[T] 1 MB file: Content-Length is 1000000");
	}

	// a slow reader must not break the server: read the big file in small steps
	{
		Conn	c;
		Reply	r;

		c.send(get("/big.txt"));
		usleep(300000);
		r = c.read();
		check(r.status == 200 && r.body.size() == 1000000, "[T] big response survives a client that starts reading late");
	}
}

/* --------------------------------- 8. routing --------------------------------- */

void	e2eTests::test_routing()
{
	section("8. ROUTING  (location matching, virtual hosts, listen forms)");

	// matchLocation: longest prefix, segment-aware
	expectNot(post("/fruits/red", "x"), 405, "[S] '/fruits' location matches '/fruits/red' (POST allowed there)");
	expect(post("/fruitsaaaa", "x"), 405, "[S] '/fruitsaaaa' does NOT match '/fruits': falls back to '/' (GET only)");
	expectNot(post("/fruits", "x"), 405, "[S] '/fruits' itself matches the '/fruits' location");
	{
		Reply	r = exchange(post("/upload/x1.txt", "x"));
		check(r.status == 201, "[S] '/upload/' (POST allowed) is chosen over '/'");
	}
	expect(post("/upload/ro/x2.txt", "x"), 405, "[S] longest prefix: '/upload/ro/' (GET only) beats '/upload/'");
	expectOpen(get("/nowhere"), 404, "[S] unmatched path falls back to the '/' location -> 404");

	// no "/" location: nothing matches -> a clean answer, not a crash
	{
		Reply	r = exchange(get("/nowhere"), false, false, 18086);
		check(r.status == 404 || r.status == 403, "[S] no '/' location and no match -> 404/403, not a crash");
		if (r.status != 404 && r.status != 403)
			std::cout << "      got: " << r.statusLine() << std::endl;
		r = exchange(get("/docs/x"), false, false, 18086);
		check(r.status == 404, "[S] server without '/' location still serves the locations it has");
	}

	// virtual hosts on 18080
	{
		Reply	a = exchange(raw("GET", "/", "HTTP/1.1", "Host: main.test\r\n"));
		Reply	b = exchange(raw("GET", "/", "HTTP/1.1", "Host: other.test\r\n"));
		Reply	c = exchange(raw("GET", "/", "HTTP/1.1", "Host: unknown.test\r\n"));
		Reply	d = exchange(raw("GET", "/", "HTTP/1.1", "Host: other.test:18080\r\n"));
		Reply	e = exchange(raw("GET", "/", "HTTP/1.1", "Host: OTHER.TEST\r\n"));

		check(a.body == readFile("www/index.html"), "[S] Host: main.test -> first server");
		check(b.body == "other vhost\n", "[S] Host: other.test -> second server (its own root)");
		check(c.body == readFile("www/index.html"), "[S] unknown Host -> first-listed (default) server");
		check(d.body == "other vhost\n", "[S] Host with ':port' still selects the server by name");
		check(e.body == "other vhost\n", "[S] Host matching is case-insensitive");
	}
	{
		Reply	r = exchange(raw("GET", "/second.html", "HTTP/1.1", "Host: other.test\r\n"));
		check(r.status == 404, "[S] second virtual host does not see the first host's files");
	}

	// every listen form really listens
	{
		const int	ports[] = {18082, 18083, 18084, 18086};
		const char	*forms[] = {"listen 18082 (port only)", "listen 127.0.0.1:18083", "listen *:18084", "listen localhost:18086"};

		for (size_t i = 0; i < sizeof(ports) / sizeof(ports[0]); i++)
		{
			Reply	r = exchange(get("/nope"), false, false, ports[i]);
			check(r.status == 404 || r.status == 200, std::string("[C] ") + forms[i] + " answers");
		}
		Reply	r = exchange(get("/index.html"), false, false, 18082);
		check(r.status == 200 && r.body == readFile("www/index.html"), "[S] each port serves its own server block (18082)");
	}
	{
		Conn	c(18081);
		check(!c.connected(), "[C] a port nobody listens on refuses the connection");
	}
	{
		Conn	a(18082);
		Conn	b(18082);
		Conn	d(E2E_PORT);
		a.send(get("/index.html"));
		b.send(get("/nope"));
		d.send(get("/second.html"));
		Reply	ra = a.read();
		Reply	rb = b.read();
		Reply	rd = d.read();
		check(ra.status == 200 && rb.status == 404 && rd.status == 200,
			"[T] simultaneous connections on two ports are served independently");
	}
}

/* -------------------- 9. redirects, autoindex, index -------------------- */

void	e2eTests::test_redirectsAutoindexIndex()
{
	section("9. REDIRECTS, AUTOINDEX, INDEX  (return / autoindex / index directives)");

	// return
	{
		Reply	r = exchange(get("/moved/"));
		check(r.status == 301, "[C] 'return 301 /dir/' -> 301");
		check(contains(r.header("location"), "/dir/"), "[C] ...with the Location header pointing at /dir/");
		check(startsWith(r.header("location"), "/") || startsWith(r.header("location"), "http"),
			"[C] ...and Location is a URL path, not a filesystem path (got '" + r.header("location") + "')");
		if (r.status != 301)
			std::cout << "      got: " << r.statusLine() << std::endl;
		r = exchange(get("/moved/anything"));
		check(r.status == 301, "[C] 'return' applies to everything under the location");
	}
	{
		Reply	r = exchange(get("/gone"));
		check(r.status == 308, "[R] 'return 308 /index.html' -> 308 Permanent Redirect");
		check(contains(r.header("location"), "/index.html"), "[R] 308 carries Location: /index.html");
		check(r.header("content-length") == "0" || r.framed, "[R] redirect response is correctly framed");
		if (r.status != 308)
			std::cout << "      got: " << r.statusLine() << std::endl;
	}
	{
		Reply	r = exchange(get("/dir"));
		check(r.status == 301 && contains(r.header("location"), "/dir/"), "[S] directory without '/' -> 301 with Location /dir/");
		r = exchange(get("/dir/"));
		check(r.status == 200 && r.body == "dir index\n", "[S] ...and with '/' serves its index.html");
		r = exchange(get("//dir"));
		check(r.status == 301 && !contains(r.header("location"), "//dir"), "[S] 301 Location uses the normalised path (no '//')");
	}

	// autoindex
	{
		Reply	r = exchange(get("/auto/"));
		check(r.status == 200 && isDir("www/auto"), "[R] autoindex on: directory listing -> 200");
		check(contains(r.body, "alpha.html") && contains(r.body, "beta.txt"), "[R] autoindex lists the directory entries");
		check(startsWith(r.header("content-type"), "text/html"), "[R] autoindex is HTML");
	}
	expectOpen(get("/noauto/"), 403, "[C] autoindex off, no index file -> 403");
	expectOpen(get("/auto"), 301, "[S] autoindex directory without '/' -> 301");

	// index
	{
		Reply	r = exchange(get("/idx/"));
		check(r.status == 200 && r.body == "idx hello\n", "[C] 'index missing.html hello.txt': first existing file wins");
	}
	{
		Reply	r = exchange(get("/fallback/"));
		check(r.status == 200 && r.body == "fallback index\n", "[S] no 'index' directive: index.html is the default");
	}
}

/* --------------------------------- 10. CGI --------------------------------- */

static std::string	bodyOf(const std::string &req)
{
	return (exchange(req).body);
}

void	e2eTests::test_cgi()
{
	section("10. CGI  (cgi_extension, environment, stdin, failures, timeout)");

	if (!_haveCgi)
	{
		std::cout << "   skipped: /usr/bin/python3 not found" << std::endl;
		return ;
	}

	{
		Reply	r = exchange(get("/cgi/hello.py"));
		check(r.status == 200 && contains(r.body, "hello from cgi"), "[C] GET script registered with cgi_extension is executed -> 200");
		check(!contains(r.body, "import sys"), "[C] the script's source is never sent");
		check(startsWith(r.header("content-type"), "text/plain"), "[T] CGI Content-Type header is passed on");
	}
	expect(get("/cgi/missing.py"), 404, "[T] CGI script that does not exist -> 404");
	{
		Reply	r = exchange(get("/cgi/hello.py?x=1"));
		check(r.status == 200 && contains(r.body, "hello from cgi"), "[T] query string does not break CGI lookup");
	}

	// environment
	{
		std::string	body = bodyOf(get("/cgi/env.py?a=1&b=two", "X-Custom: custom-value\r\n"));
		check(contains(body, "REQUEST_METHOD=GET"), "[T] CGI env: REQUEST_METHOD");
		check(contains(body, "QUERY_STRING=a=1&b=two"), "[T] CGI env: QUERY_STRING");
		check(contains(body, "SERVER_PROTOCOL=HTTP/1.1"), "[T] CGI env: SERVER_PROTOCOL");
		check(contains(body, "HTTP_X_CUSTOM=custom-value"), "[T] CGI env: request headers become HTTP_*");
		body = bodyOf(post("/cgi/env.py", "12345"));
		check(contains(body, "REQUEST_METHOD=POST") && contains(body, "CONTENT_LENGTH=5"), "[T] CGI env on POST: METHOD and CONTENT_LENGTH");
	}

	// stdin
	{
		Reply	r = exchange(post("/cgi/upper.py", "hello world"));
		check(r.status == 200 && r.body == "HELLO WORLD", "[T] POST body reaches the script's stdin");
		r = exchange(postChunked("/cgi/upper.py", chunk("hello") + chunk(" world") + "0\r\n\r\n"));
		check(r.status == 200 && r.body == "HELLO WORLD", "[P] chunked request body is un-chunked before it reaches the script");
		r = exchange(post("/cgi/upper.py", ""));
		check(r.status == 200 && r.body.empty(), "[T] POST with empty body -> script runs, empty answer");

		std::string	big(200000, 'q');
		r = exchange(post("/cgi/upper.py", big));
		check(r.status == 200 && r.body == std::string(200000, 'Q'), "[T] 200 KB through the CGI pipes (no deadlock, nothing lost)");
		r = exchange(post("/cgi/upper.py", std::string(1000001, 'x')));
		check(r.status == 413, "[S] CGI location's client_max_body_size (1000000) is enforced");
	}

	// failures
	expect(get("/cgi/fail.py"), 500, "[T] script exits non-zero without output -> 500");
	expect(get("/cgi/crash.py"), 500, "[T] script raises an exception -> 500");
	expectOpen(get("/cgi/hello.py"), 200, "[T] server keeps serving CGI after failed scripts");

	// several scripts at once
	{
		Conn	a, b, c;
		a.send(get("/cgi/hello.py"));
		b.send(post("/cgi/upper.py", "abc"));
		c.send(get("/cgi/env.py"));
		Reply	ra = a.read();
		Reply	rb = b.read();
		Reply	rc = c.read();
		check(ra.status == 200 && rb.body == "ABC" && rc.status == 200, "[T] three CGI requests in flight at once");
	}

	// non-CGI request type on the CGI location still works
	check(exchange(get("/cgi/")).status == 403, "[T] directory inside the CGI location -> 403");

	// no zombies left behind
	{
		int	children, zombies;

		usleep(200000);
		countChildren(_server.pid(), children, zombies);
		check(zombies == 0, "[T] no zombie processes after the CGI requests above");
		check(children == 0, "[T] no CGI child left running after the requests above");
	}

	// hang -> 504, and the stuck child must be killed
	if (std::getenv("E2E_SKIP_SLOW"))
		std::cout << "   skipped (E2E_SKIP_SLOW set): CGI timeout" << std::endl;
	else
	{
		Conn	c;
		Reply	r;
		int		children, zombies;

		c.send(get("/cgi/hang.py"));
		r = c.read(false, 20000);
		check(r.status == 504, "[T] script that never answers -> 504 after the CGI timeout (~10 s)");
		if (r.status != 504)
			std::cout << "      got: " << r.statusLine() << std::endl;
		usleep(300000);
		countChildren(_server.pid(), children, zombies);
		check(children == 0 && zombies == 0, "[T] the timed-out script is killed and reaped");
		expectOpen(get("/index.html"), 200, "[T] server is responsive after a CGI timeout");
	}
}
