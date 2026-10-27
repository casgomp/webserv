# include <cstdlib>
# include <iostream>
# include <unistd.h>

# include "e2eTests.hpp"

using namespace e2e;

# define E2E_CONF	"e2e.conf"
# define E2E_LOG	"e2e_server.log"

e2eTests::e2eTests() : TestSuite("e2eTests"), _isRoot(geteuid() == 0),
	_haveCgi(access("/usr/bin/python3", X_OK) == 0) {}

/* ============================ infrastructure ============================ */

void	e2eTests::section(const std::string &title)
{
	std::cout << "\n\033[1m== " << title << " ==\033[0m" << std::endl;
}

bool	e2eTests::startMainServer()
{
	return (_server.start(E2E_CONF, E2E_PORT, E2E_LOG));
}

// After every category: a crash is reported once, against the category that caused it.
void	e2eTests::alive(const std::string &after)
{
	bool	up = _server.running();

	check(up, "server still running after: " + after
		+ (up ? "" : " (exit code " + toStr(_server.lastExit()) + ", see e2e_server.log)"));
	if (!up)
		startMainServer();
}

static void	why(const Reply &r)
{
	std::cout << "      got: " << r.statusLine() << (r.closed ? "  [connection closed]" : "") << std::endl;
}

// status only
void	e2eTests::expect(const std::string &req, int status, const std::string &msg)
{
	Reply	r = exchange(req);
	bool	ok = (r.status == status);

	check(ok, msg);
	if (!ok)
		why(r);
}

// status AND the connection stays usable (fully-read request)
void	e2eTests::expectOpen(const std::string &req, int status, const std::string &msg)
{
	Reply	r = exchange(req, false, true);
	bool	ok = (r.status == status && !r.closed);

	check(ok, msg + " (connection stays open)");
	if (!ok)
		why(r);
}

// status AND the server hangs up (request that could not be parsed / was refused unread)
void	e2eTests::expectClosed(const std::string &req, int status, const std::string &msg)
{
	Reply	r = exchange(req, false, true);
	bool	ok = (r.status == status && r.closed);

	check(ok, msg + " (connection closed)");
	if (!ok)
		why(r);
}

// any status except the given one (used where several answers are legitimate)
void	e2eTests::expectNot(const std::string &req, int status, const std::string &msg)
{
	Reply	r = exchange(req);
	bool	ok = (r.status != 0 && r.status != status);

	check(ok, msg);
	if (!ok)
		why(r);
}

// Incomplete request: the server must wait (no reply), then answer once the rest arrives.
void	e2eTests::dribble(const std::string &first, const std::string &second,
			int status, const std::string &msg)
{
	Conn	c;
	bool	waited;
	Reply	r;

	c.send(first);
	waited = c.silent(250);
	c.send(second);
	r = c.read();
	check(waited && r.status == status, msg + " (waits for the rest, then answers)");
	if (!waited || r.status != status)
		why(r);
}

/* =============================== fixtures =============================== */

void	e2eTests::setupFixtures()
{
	std::string	bin;
	std::string	big;

	for (int i = 0; i < 1024; i++)
		bin += static_cast<char>(i % 256);
	for (int i = 0; i < 1000000; i++)
		big += static_cast<char>('a' + i % 26);

	removeTree("www");
	removeTree("www2");
	mkdirs("www/upload/ro");
	mkdirs("www/all");
	mkdirs("www/tiny");
	mkdirs("www/auto");
	mkdirs("www/noauto");
	mkdirs("www/idx");
	mkdirs("www/dir");
	mkdirs("www/fallback");
	mkdirs("www/cgi");
	mkdirs("www2");

	writeFile("www/index.html", "<html><body>e2e</body></html>\n");
	writeFile("www/second.html", "second page\n");
	writeFile("www/hello world.txt", "name with a space\n");
	writeFile("www/a.txt", "plain text\n");
	writeFile("www/a.json", "{\"a\":1}\n");
	writeFile("www/a.css", "body{}\n");
	writeFile("www/a.mp4", bin);
	writeFile("www/binary.bin", bin);
	writeFile("www/big.txt", big);
	writeFile("www/dir/index.html", "dir index\n");
	writeFile("www/auto/alpha.html", "alpha\n");
	writeFile("www/auto/beta.txt", "beta\n");
	writeFile("www/idx/hello.txt", "idx hello\n");
	writeFile("www/fallback/index.html", "fallback index\n");
	writeFile("www2/index.html", "other vhost\n");

	writeFile("www/cgi/hello.py",
		"import sys\n"
		"sys.stdout.write('Content-Type: text/plain\\r\\n\\r\\nhello from cgi\\n')\n");
	writeFile("www/cgi/env.py",
		"import os, sys\n"
		"sys.stdout.write('Content-Type: text/plain\\r\\n\\r\\n')\n"
		"for k in ['REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_LENGTH', 'SERVER_PROTOCOL', 'HTTP_X_CUSTOM']:\n"
		"    sys.stdout.write('%s=%s\\n' % (k, os.environ.get(k, '<unset>')))\n");
	writeFile("www/cgi/upper.py",
		"import sys\n"
		"data = sys.stdin.buffer.read()\n"
		"sys.stdout.buffer.write(b'Content-Type: text/plain\\r\\n\\r\\n' + data.upper())\n");
	writeFile("www/cgi/fail.py", "import sys\nsys.exit(1)\n");
	writeFile("www/cgi/crash.py", "raise Exception('boom')\n");
	writeFile("www/cgi/hang.py", "import time\ntime.sleep(60)\n");
}

void	e2eTests::cleanupFixtures()
{
	removeTree("www");
	removeTree("www2");
	removeTree("e2e_tmp.conf");
}

/* ================================= run ================================= */

void	e2eTests::run_all()
{
	std::cout << "\n\033[30;105mRunning e2eTests...\033[0m\n" << std::endl;

	setupFixtures();
	if (!startMainServer())
	{
		check(false, "server starts and accepts connections (see e2e_server.log)");
		cleanupFixtures();
		printSummary();
		return ;
	}

	test_responseFormat();			alive("1. response format");
	test_requestLine();				alive("2. request line");
	test_headersAndFraming();		alive("3. headers and framing");
	test_body();					alive("4. body");
	test_connection();				alive("5. connection handling");
	test_methods();					alive("6. methods");
	test_pathsAndStaticFiles();		alive("7. paths and static files");
	test_routing();					alive("8. routing");
	test_redirectsAutoindexIndex();	alive("9. redirects, autoindex, index");
	test_cgi();						alive("10. CGI");

	test_config();					// spawns its own short-lived servers

	section("12. SHUTDOWN");
	check(_server.stop(), "[T] server exits on SIGINT within 2 s");

	cleanupFixtures();
	printSummary();
}
