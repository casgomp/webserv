# include <iostream>
# include <unistd.h>

# include "e2eTests.hpp"

using namespace e2e;

/*
** Category 11: configuration. Every case starts its own short-lived ./webserv with a
** generated config on port 18099 (ConfigParserTests, ServerTests: serverInit/getListenServers).
*/

# define CONF_PORT	18099
# define CONF_FILE	"e2e_tmp.conf"
# define CONF_LOG	"e2e_conf.log"

static std::string	indent(const std::string &lines, const std::string &pad)
{
	std::string	out;
	size_t		start = 0;

	while (start < lines.size())
	{
		size_t	nl = lines.find('\n', start);
		if (nl == std::string::npos)
			nl = lines.size();
		out += pad + lines.substr(start, nl - start) + "\n";
		start = nl + 1;
	}
	return (out);
}

// One server on CONF_PORT with a "/" location; each extra directive goes on its own line.
static std::string	confWith(const std::string &serverLines, const std::string &locationLines)
{
	return (std::string("http {\n    include mime.types;\n    root www;\n    server {\n")
		+ "        listen localhost:" + toStr(CONF_PORT) + ";\n"
		+ indent(serverLines, "        ")
		+ "        location / {\n            allowed_methods GET;\n"
		+ indent(locationLines, "            ")
		+ "        }\n    }\n}\n");
}

// true if webserv refuses the config: exits by itself, with an error code, not a crash
static bool	rejects(const std::string &conf, int &code)
{
	ServerProc	p;

	writeFile(CONF_FILE, conf);
	if (!p.start(CONF_FILE, 0, CONF_LOG))
		return (false);
	code = p.waitExit(2500);
	if (code == -1)
	{
		p.stop();
		return (false);
	}
	return (code > 0 && code < 128);
}

// true if webserv starts and answers on CONF_PORT (plus every extra port given)
static bool	accepts(const std::string &conf, int extraPort = 0)
{
	ServerProc	p;
	bool		ok;

	writeFile(CONF_FILE, conf);
	ok = p.start(CONF_FILE, CONF_PORT, CONF_LOG);
	if (ok && extraPort)
	{
		Conn	c(extraPort);
		ok = c.connected();
	}
	p.stop();
	return (ok);
}

void	e2eTests::test_config()
{
	section("11. CONFIGURATION  (valid and invalid config files)");

	int	code = 0;

	mkdirs("www");
	// ---- the config file itself ----
	{
		ServerProc	p;

		p.start("does_not_exist.conf", 0, CONF_LOG);
		code = p.waitExit(2500);
		check(code > 0 && code < 128, "[C] missing config file -> error exit (no crash)");
		if (code == -1)
			p.stop();
	}

	// ---- location-level directives: invalid ----
	struct { const char *directive; const char *what; }	badLocation[] = {
		{"cgi_extension py /usr/bin/python3;", "cgi_extension without leading dot"},
		{"cgi_extension .sh;", "cgi_extension with a single argument"},
		{"cgi_extension .sh /bin/sh extra;", "cgi_extension with three arguments"},
		{"cgi_extension ;", "empty cgi_extension"},
		{"return ;", "empty return"},
		{"return 200 /new-page;", "return with a non-redirect status code"},
		{"allowed_methods ;", "empty allowed_methods"},
		{"allowed_methods GET PUT;", "allowed_methods with an unsupported method"},
		{"autoindex garbage;", "autoindex with a value other than on/off"},
		{"index ;", "empty index"}};

	for (size_t i = 0; i < sizeof(badLocation) / sizeof(badLocation[0]); i++)
	{
		bool	bad = rejects(confWith("", badLocation[i].directive), code);

		check(bad, std::string("[C] rejected: ") + badLocation[i].what);
		if (!bad)
			std::cout << "      exit code: " << code << " (-1 = server kept running)" << std::endl;
	}

	// ---- server-level directives: invalid ----
	struct { const char *directive; const char *what; }	badServer[] = {
		{"listen ;", "empty listen"},
		{"listen localhost:*;", "listen with a non-numeric port"},
		{"server_name ;", "empty server_name"}};

	for (size_t i = 0; i < sizeof(badServer) / sizeof(badServer[0]); i++)
	{
		bool	bad = rejects(confWith(badServer[i].directive, ""), code);

		check(bad, std::string("[C] rejected: ") + badServer[i].what);
		if (!bad)
			std::cout << "      exit code: " << code << " (-1 = server kept running)" << std::endl;
	}

	// ---- valid forms ----
	check(accepts(confWith("", "cgi_extension .py /usr/bin/python3;\ncgi_extension .php /usr/bin/php-cgi;")),
		"[C] accepted: two cgi_extension lines in one location");
	check(accepts(confWith("", "cgi_extension .py /usr/bin/python3;\ncgi_extension .py /usr/local/bin/python3;")),
		"[C] accepted: repeated cgi_extension overrides the interpreter");
	check(accepts(confWith("", "return 301 /new-page;")), "[C] accepted: return 301 /new-page");
	check(accepts(confWith("", "autoindex on;")), "[C] accepted: autoindex on");
	check(accepts(confWith("", "autoindex off;")), "[C] accepted: autoindex off");
	check(accepts(confWith("", "index index index.html index.txt fruits.html;")), "[C] accepted: index with several files");
	check(accepts(confWith("server_name mysite.com www.mysite.com;", "")), "[C] accepted: several server_name values");
	check(accepts(confWith("listen 18097;", ""), 18097), "[C] accepted: a second listen in the same server (both ports answer)");
	check(accepts(confWith("", "allowed_methods GET POST DELETE;")), "[C] accepted: allowed_methods GET POST DELETE");

	// ---- server_name routing from a generated config ----
	{
		ServerProc	p;
		writeFile(CONF_FILE, confWith("server_name mysite.com www.mysite.com;", ""));
		if (p.start(CONF_FILE, CONF_PORT, CONF_LOG))
		{
			Reply	r = exchange(raw("GET", "/index.html", "HTTP/1.1", "Host: www.mysite.com\r\n"), false, false, CONF_PORT);
			check(r.status == 200, "[S] every name listed in server_name is served");
		}
		else
			check(false, "[S] server with server_name starts");
		p.stop();
	}

	// ---- bind conflicts (serverInit): same port, different spelling of the same address ----
	{
		std::string	two = "http {\n    root www;\n"
			"    server {\n        listen localhost:18098;\n        location / {\n            allowed_methods GET;\n        }\n    }\n"
			"    server {\n        listen 127.0.0.1:18098;\n        location / {\n            allowed_methods GET;\n        }\n    }\n}\n";
		bool	bad = rejects(two, code);

		check(bad, "[S] rejected: 'localhost:P' and '127.0.0.1:P' in two servers (same address:port)");
		if (!bad)
			std::cout << "      exit code: " << code << " (-1 = server kept running)" << std::endl;

		std::string	wild = "http {\n    root www;\n"
			"    server {\n        listen *:18098;\n        location / {\n            allowed_methods GET;\n        }\n    }\n"
			"    server {\n        listen 127.0.0.1:18098;\n        location / {\n            allowed_methods GET;\n        }\n    }\n}\n";
		bad = rejects(wild, code);
		check(bad, "[S] rejected: '*:P' together with '127.0.0.1:P'");
		if (!bad)
			std::cout << "      exit code: " << code << " (-1 = server kept running)" << std::endl;
	}

	// ---- a port that is already taken ----
	{
		ServerProc	first;
		ServerProc	second;
		int			c2;

		writeFile(CONF_FILE, confWith("", ""));
		if (first.start(CONF_FILE, CONF_PORT, CONF_LOG))
		{
			second.start(CONF_FILE, 0, "e2e_conf2.log");
			c2 = second.waitExit(2500);
			check(c2 > 0 && c2 < 128, "[S] second instance on a taken port exits with an error (no crash)");
			if (c2 == -1)
				second.stop();
		}
		else
			check(false, "[S] first instance starts");
		first.stop();
		removeTree("e2e_conf2.log");
	}

	removeTree(CONF_FILE);
	removeTree(CONF_LOG);
}
