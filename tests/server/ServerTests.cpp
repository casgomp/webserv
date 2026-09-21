/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerTests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 11:29:09 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/06 15:32:00 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerTests.hpp"
#include "../../include/webserv.hpp"

//request validation all util functions and request validation itself

ServerTests::ServerTests() : TestSuite("ServerTests") {}

void	ServerTests::test_getValidMimeTypes()
{
	std::map<std::string, std::string>	mime = getValidMimeTypes();

	check(!mime.empty(), "getValidMimeTypes returns a non-empty map");
	check(mime["html"] == "text/html", "getValidMimeTypes mime.[html] = text/html");
	check(mime["txt"] == "text/plain", "getValidMimeTypes mime.[txt] = text/plain");
	check(mime["mp4"] == "video/mp4", "getValidMimeTypes mime.[mp4] = video/mp4");
}

void	ServerTests::test_getContentType()
{
	std::string	type1 = getContentType("file.html");
	std::string	type2 = getContentType("content/file.txt");
	std::string	type3 = getContentType("content.user/file.user.date.json");

	check(type1 == "text/html", "getContentType returns correct content-type for file path");
	check(type2 == "text/plain", "getContentType returns correct content-type for dir/file path");
	check(type3 == "application/json", "getContentType returns correct content-type for path with multiple dots");
}

void    ServerTests::test_pathIsDir()
{
	std::ofstream tmpFile("test_pathIsDir.tmp");
	tmpFile << "test content";
	tmpFile.close();

	check(pathIsDir(".") == true,
		"pathIsDir returns true for an existing directory");

	check(pathIsDir("test_pathIsDir.tmp") == false,
		"pathIsDir returns false for a regular file");

	check(pathIsDir("no_such_dir_xyz") == false,
		"pathIsDir returns false for a nonexistent path");

	std::remove("test_pathIsDir.tmp");
}

void    ServerTests::test_pathIsFile()
{
	std::ofstream tmpFile("test_pathIsFile.tmp");
	tmpFile << "test content";
	tmpFile.close();

	check(pathIsFile("test_pathIsFile.tmp") == true,
		"pathIsFile returns true for an existing regular file");

	check(pathIsFile(".") == false,
		"pathIsFile returns false for a directory");

	check(pathIsFile("no_such_file_xyz.tmp") == false,
		"pathIsFile returns false for a nonexistent path");

	std::remove("test_pathIsFile.tmp");
}

void    ServerTests::test_joinedPath()
{
	check(joinedPath("content/", "/docs/file.txt") == "content/docs/file.txt",
		"joinedPath strips root's trailing slash and target's leading slash");

	check(joinedPath("content", "docs/file.txt") == "content/docs/file.txt",
		"joinedPath joins correctly when neither side has a boundary slash");

	check(joinedPath("content/", "") == "content/",
		"joinedPath handles an empty target, leaving just root + trailing slash");

	check(joinedPath("content/", "/") == "content/",
		"joinedPath handles a bare '/' target the same as an empty target");
}

void	ServerTests::test_normalizePath()
{
	std::string normalizedTarget;

	normalizePath("/docs/../private/./file.txt", normalizedTarget);
	check(normalizedTarget == "/private/file.txt",
		"normalizePath resolves '..' and strips '.' segments");

	normalizePath("/docs//private", normalizedTarget);
	check(normalizedTarget == "/docs/private",
		"normalizePath collapses double slashes");

	normalizePath("/../../etc/passwd", normalizedTarget);
	check(normalizedTarget == "",
		"normalizePath returns empty string when traversal goes past root");

	normalizePath("/", normalizedTarget);
	check(normalizedTarget == "/",
		"normalizePath handles root target");
}

void	ServerTests::test_createRedirectPath()
{
	t_locationConf	location;

	location.root = "content/";

	location.redirection.second = "/new-page";
	check(createRedirectPath(&location) == "content/new-page",
		"createRedirectPath prepends root when target starts with '/'");

	location.redirection.second = "example";
	check(createRedirectPath(&location) == "example",
		"createRedirectPath uses target as-is when it doesn't start with '/'");
}

void	ServerTests::test_validateMethod()
{
	t_locationConf				location;
	std::vector<std::string>	allowedMethods;

	allowedMethods.push_back("GET");
	allowedMethods.push_back("DELETE");
	location.allowedMethods = allowedMethods;
	check(validateMethod(location.allowedMethods, "GET") == 0, "validateMethod returns 0 if request method is allowed in location");
	check(validateMethod(location.allowedMethods, "PIZZA") == 405, "validateMethod returns 405 if request method is invalid");
	check(validateMethod(location.allowedMethods, "POST") == 403, "validateMethod returns 403 if request method is now allowed in location");
}

void	ServerTests::test_matchLocation()
{
	std::string target = "/docs/private/file.txt";
	std::vector<t_locationConf> locations;

	t_locationConf root;
	root.path = "/";
	locations.push_back(root);

	t_locationConf docs;
	docs.path = "/docs";
	locations.push_back(docs);

	t_locationConf docsPrivate;
	docsPrivate.path = "/docs/private";
	locations.push_back(docsPrivate);

	t_locationConf fruits;
	fruits.path = "/fruits";
	locations.push_back(fruits);

	t_locationConf *result = matchLocation(locations, target);
	check (result != NULL && result->path == "/docs/private", "matchLocation chooses the longest matching prefix");

	result = matchLocation(locations, "/docs/readme.md");
	check(result != NULL && result->path == "/docs",
		"matchLocation falls back to shorter prefix when longer one doesn't match");

	result = matchLocation(locations, "/");
	check(result != NULL && result->path == "/",
		"matchLocation matches root exactly");

	result = matchLocation(locations, "/nowhere");
	check(result != NULL && result->path == "/",
		"matchLocation falls back to root location when nothing more specific matches");

	result = matchLocation(locations, "/fruitsaaaa");
	check(result != NULL && result->path == "/",
		"matchLocation does not let /fruitsaaaa falsely match /fruits, falls back to root");

	result = matchLocation(locations, "/fruits");
	check(result != NULL && result->path == "/fruits",
		"matchLocation matches /fruits exactly");

	result = matchLocation(locations, "/fruits/red");
	check(result != NULL && result->path == "/fruits",
		"matchLocation matches /fruits as prefix of a real sub-path");

	std::vector<t_locationConf> noRootLocations;
	noRootLocations.push_back(docs);
	noRootLocations.push_back(fruits);

	result = matchLocation(noRootLocations, "/nowhere");
	check(result == NULL,
		"matchLocation returns NULL when nothing matches and there is no root location");
}

void	ServerTests::test_requestRouting()
{
	t_serverConf	server0;
	t_serverConf	server1;
	server0.serverNames.push_back("mysite.com");
	server0.root = "root0";
	server1.serverNames.push_back("mayonesa.com");
	server1.root = "root1";

	t_listenServers listenServers;
	std::pair<std::string, std::string> pair = std::make_pair("localhost", "8080");
	listenServers[pair].push_back(&server0);
	listenServers[pair].push_back(&server1);

	std::map<int, t_client> clients;
	int fd = 5;
	clients[fd].pairAddressPort = pair;

	HttpRequest httpRequest1;
	httpRequest1.headers["host"] = "mayonesa.com";
	requestRouting(fd, clients, listenServers, httpRequest1);
	check(clients[fd].serverConf == &server1, "request routing: routes to server1 when Host matches server1's name");

	HttpRequest httpRequest2;
	httpRequest1.headers["host"] = "nonexistent.com";
	requestRouting(fd, clients, listenServers, httpRequest2);
	check(clients[fd].serverConf == &server0, "request routing: falls back to first-listed server when Host matches nothing");
}

void	ServerTests::test_serverInit()
{
	t_httpConf					httpConf1;
	t_httpConf					httpConf2;
	std::vector<t_serverConf>	vecServerConf1;
	std::vector<t_serverConf>	vecServerConf2;
	t_serverConf				server0;
	t_serverConf				server1;
	t_serverConf				server2;
	t_serverConf				server3;
	t_serverConf				server4;
	t_serverConf				server5;
	t_serverConf				server6;
	t_serverConf				server7;
	t_listenServers				listenServers1;
	t_listenServers				listenServers2;
	t_listeningSockets			listeningSockets1;
	t_listeningSockets			listeningSockets2;
	bool						success = true;
	bool						fail = false;

	server0.listen.push_back(std::make_pair("*", "8080"));
	server1.listen.push_back(std::make_pair("127.0.0.1", "5173"));
	server2.listen.push_back(std::make_pair("127.0.0.1", "5173"));
	server3.listen.push_back(std::make_pair("localhost", "8081"));

	server4.listen.push_back(std::make_pair("*", "5174"));
	server5.listen.push_back(std::make_pair("127.0.0.1", "5174"));
	server6.listen.push_back(std::make_pair("127.0.0.1", "8080"));
	server7.listen.push_back(std::make_pair("localhost", "8080"));

	vecServerConf1.push_back(server0);
	vecServerConf1.push_back(server1);
	vecServerConf1.push_back(server2);
	vecServerConf1.push_back(server3);
	httpConf1.servers = vecServerConf1;

	vecServerConf2.push_back(server4);
	vecServerConf2.push_back(server5);
	vecServerConf2.push_back(server6);
	vecServerConf2.push_back(server7);
	httpConf2.servers = vecServerConf2;

	listenServers1 = getListenServers(httpConf1);
	try {
		listeningSockets1 = serverInit(listenServers1);
	} catch (const std::exception &e) {
		success = false;
	}
	for (t_listeningSockets::iterator it = listeningSockets1.begin(); it != listeningSockets1.end(); it ++)
		close(it->first);

	listenServers2 = getListenServers(httpConf2);
	try {
		listeningSockets2 = serverInit(listenServers2);
	} catch (const std::exception &e) {
		fail = true;
	}
	for (t_listeningSockets::iterator it = listeningSockets2.begin(); it != listeningSockets2.end(); it ++)
		close(it->first);

	check(success && listeningSockets1.size() == 3, "listening sockets contains 1 fd per unique addres:port pair");
	check(fail, "listening sockets fail due to non-unique pair (same address but different naming, same port)");
}

void	ServerTests::test_getListenServers()
{
	t_httpConf					httpConf;
	std::vector<t_serverConf>	vecServerConf;
	t_serverConf				server0;
	t_serverConf				server1;
	t_serverConf				server2;
	t_serverConf				server3;
	t_listenServers				listServ;

	server0.listen.push_back(std::make_pair("localhost", "8080"));
	server0.serverNames.push_back("myserver.com");

	server1.listen.push_back(std::make_pair("*", "8080"));
	server1.serverNames.push_back("www.website.com");

	server2.listen.push_back(std::make_pair("127.0.0.1", "80"));
	server2.serverNames.push_back("www.hello.com");

	server3.listen.push_back(std::make_pair("127.0.0.1", "80"));
	server3.serverNames.push_back("something.com");

	vecServerConf.push_back(server0);
	vecServerConf.push_back(server1);
	vecServerConf.push_back(server2);
	vecServerConf.push_back(server3);
	httpConf.servers = vecServerConf;

	listServ = getListenServers(httpConf);

	check(listServ.size() == 3, "getListenServers produces 3 distinct address:port keys");

	std::pair<std::string, std::string> keyLocalhost = std::make_pair("localhost", "8080");
	std::pair<std::string, std::string> keyWildcard = std::make_pair("", "8080");
	std::pair<std::string, std::string> keyLoopback = std::make_pair("127.0.0.1", "80");

	check(listServ.find(keyLocalhost) != listServ.end(), "map contains localhost:8080 key");
	check(listServ.find(keyWildcard) != listServ.end(), "map contains '' (normalized from '*'):8080 key");
	check(listServ.find(keyLoopback) != listServ.end(), "map contains 127.0.0.1:80 key");

	check(listServ[keyLocalhost].size() == 1, "localhost:8080 has exactly 1 server");
	check(listServ[keyWildcard].size() == 1, "wildcard:8080 has exactly 1 server");
	check(listServ[keyLoopback].size() == 2, "127.0.0.1:80 has exactly 2 servers sharing the port");

	check(listServ[keyLocalhost].at(0)->serverNames.at(0) == "myserver.com", "localhost:8080 maps to correct server");
	check(listServ[keyLoopback].at(0)->serverNames.at(0) == "www.hello.com", "127.0.0.1:80 first entry is correct server");
	check(listServ[keyLoopback].at(1)->serverNames.at(0) == "something.com", "127.0.0.1:80 second entry is correct server");
}

void	ServerTests::run_all()
{
	std::cout << "\n\033[30;105mRunning ServerTests...\033[0m\n" << std::endl;

	test_getListenServers();
	test_serverInit();
	test_requestRouting();
	test_matchLocation();
	test_validateMethod();
	test_createRedirectPath();
	test_normalizePath();
	test_joinedPath();
	test_pathIsFile();
	test_pathIsDir();
	test_getContentType();
	test_getValidMimeTypes();

	printSummary();
}