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

ServerTests::ServerTests() : TestSuite("ServerTests") {}

void	ServerTests::test_requestParsing()
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
	test_requestParsing();

	printSummary();
}