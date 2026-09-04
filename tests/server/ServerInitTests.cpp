/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInitTests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 11:29:09 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/04 17:14:04 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerInitTests.hpp"
#include "../../include/webserv.hpp"

ServerInitTests::ServerInitTests() : TestSuite("ServerInitTests") {}

void	ServerInitTests::test_getListeningServers()
{
	t_httpConf					httpConf;
	std::vector<t_serverConf>	vecServerConf;
	t_serverConf				server0;
	t_serverConf				server1;
	t_serverConf				server2;
	t_serverConf				server3;

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

	std::map<std::pair<std::string, std::string>, std::vector<t_serverConf *> > listServ;
	listServ = getListeningServers(httpConf);

	check(listServ.size() == 3, "getListeningServers produces 3 distinct address:port keys");

	std::pair<std::string, std::string> keyLocalhost = std::make_pair("localhost", "8080");
	std::pair<std::string, std::string> keyWildcard = std::make_pair("", "8080");
	std::pair<std::string, std::string> keyLoopback = std::make_pair("127.0.0.1", "80");

	check(listServ.find(keyLocalhost) != listServ.end(), "map contains localhost:8080 key");
	check(listServ.find(keyWildcard) != listServ.end(), "map contains '' (normalized from '*'):8080 key");
	check(listServ.find(keyLoopback) != listServ.end(), "map contains 127.0.0.1:80 key");

	check(listServ[keyLocalhost].size() == 1, "localhost:8080 has exactly 1 server");
	check(listServ[keyWildcard].size() == 1, "wildcard:8080 has exactly 1 server");
	check(listServ[keyLoopback].size() == 2, "127.0.0.1:80 has exactly 2 servers sharing the port");

	check(listServ[keyLocalhost].at(0)->serverNames.at(0) == "myserver.com",
		"localhost:8080 maps to correct server");
	check(listServ[keyLoopback].at(0)->serverNames.at(0) == "www.hello.com",
		"127.0.0.1:80 first entry is correct server");
	check(listServ[keyLoopback].at(1)->serverNames.at(0) == "something.com",
		"127.0.0.1:80 second entry is correct server");
}

void	ServerInitTests::run_all()
{
	std::cout << "\n\033[30;105mRunning ServerInitTests...\033[0m\n" << std::endl;

	test_getListeningServers();

	printSummary();
}