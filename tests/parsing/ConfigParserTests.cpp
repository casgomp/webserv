/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserTests.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 11:29:09 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/03 18:04:27 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParserTests.hpp"
#include "../../include/webserv.hpp"

ConfigParserTests::ConfigParserTests() : TestSuite("ConfigParserTests") {}

void	ConfigParserTests::test_addRedirection()
{
	t_locationConf	locationConf1;
	t_locationConf	locationConf2;
	bool			success = true;
	bool			failEmpty = false;
	bool			failBadUri = false;
	bool			failBadCode = false;

	try {
		addRedirection(locationConf1, "301 /new-page");
	} catch (const std::exception &e) {
		success = false;
	}
	try {
		addRedirection(locationConf2, "");
	} catch (const std::exception &e) {
		failEmpty = true;
	}
	try {
		addRedirection(locationConf2, "301 new-page");
	} catch (const std::exception &e) {
		failBadUri = true;
	}
	try {
		addRedirection(locationConf2, "200 /new-page");
	} catch (const std::exception &e) {
		failBadCode = true;
	}

	check(success, "addRedirection accepts a valid input without throwing");
	check(failEmpty, "addRedirection throws on empty string");
	check(failBadUri, "addRedirection throws when path does not start with '/'");
	check(failBadCode, "addRedirection throws on out-of-range status code");

	check(locationConf1.redirection.first == 301, "addRedirection stores correct status code");
	check(locationConf1.redirection.second == "/new-page", "addRedirection stores correct target path");
}

void	ConfigParserTests::test_addAllowedMethods()
{
	t_locationConf	locationConf1;
	t_locationConf	locationConf2;
	bool			success = true;
	bool			failEmpty = false;
	bool			failInvalid = false;

	try {
		addAllowedMethods(locationConf1, "GET POST");
	} catch (const std::exception &e) {
		success = false;
	}
	try {
		addAllowedMethods(locationConf2, "");
	} catch (const std::exception &e) {
		failEmpty = true;
	}
	try {
		addAllowedMethods(locationConf2, "GET PUT");
	} catch (const std::exception &e) {
		failInvalid = true;
	}

	check(success, "addAllowedMethods accepts a valid method list without throwing");
	check(failEmpty, "addAllowedMethods throws on empty string");
	check(failInvalid, "addAllowedMethods throws on an unsupported method");

	check(locationConf1.allowedMethods.size() == 2,
		"addAllowedMethods stores exactly the methods provided, not the constructor defaults");
	check(locationConf1.allowedMethods.at(0) == "GET", "addAllowedMethods stores first method correctly");
	check(locationConf1.allowedMethods.at(1) == "POST", "addAllowedMethods stores second method correctly");
}

void	ConfigParserTests::test_addErrorPages()
{
	t_serverConf	serverConf1;
	t_serverConf	serverConf2;
	bool			success = true;
	bool			failEmpty = false;
	bool			failBadUri = false;
	bool			failBadCode = false;

	try {
		addErrorPages(serverConf1, "404 /404.html");
		addErrorPages(serverConf2, "500 502 503 /50x.html");
	} catch (const std::exception &e) {
		success = false;
	}
	try {
		addErrorPages(serverConf1, "");
	} catch (const std::exception &e) {
		failEmpty = true;
	}
	try {
		addErrorPages(serverConf1, "404 404.html");
	} catch (const std::exception &e) {
		failBadUri = true;
	}
	try {
		addErrorPages(serverConf1, "999 /404.html");
	} catch (const std::exception &e) {
		failBadCode = true;
	}

	check(success, "addErrorPages accepts valid single and multi-code inputs without throwing");
	check(failEmpty, "addErrorPages throws on empty string");
	check(failBadUri, "addErrorPages throws when path does not start with '/'");
	check(failBadCode, "addErrorPages throws on out-of-range status code");

	check(serverConf1.errorPages.size() == 1, "single-code error_page stores one entry");
	check(serverConf1.errorPages[404] == "/404.html", "single-code error_page maps 404 to correct path");

	check(serverConf2.errorPages.size() == 3, "multi-code error_page stores one entry per code");
	check(serverConf2.errorPages[500] == "/50x.html", "multi-code error_page maps 500 correctly");
	check(serverConf2.errorPages[502] == "/50x.html", "multi-code error_page maps 502 correctly");
	check(serverConf2.errorPages[503] == "/50x.html", "multi-code error_page maps 503 correctly");
}

void	ConfigParserTests::test_addListenAddressPort()
{
	t_serverConf	serverConf;
	bool			success = true;
	bool			fail = false;

	try {
		addListenAddressPort(serverConf, "127.0.0.1:8000");
		addListenAddressPort(serverConf, "127.0.0.1");
		addListenAddressPort(serverConf, "8000");
		addListenAddressPort(serverConf, "*:8000");
		addListenAddressPort(serverConf, "localhost:8000");
	} catch (const std::exception &e) {
		success = false;
	}
	try {
		addListenAddressPort(serverConf, "");
	} catch (const std::exception &e) {
		fail = true;
	}

	check(success, "addListenAddressPort accepts all valid listen forms without throwing");
	check(fail, "addListenAddressPort throws on empty string");
	check(serverConf.listen.size() == 5, "addListenAddressPort stores correct number of entries");

	check(serverConf.listen.at(0).first == "127.0.0.1" && serverConf.listen.at(0).second == "8000",
		"listen '127.0.0.1:8000' splits into correct address and port");
	check(serverConf.listen.at(1).first == "127.0.0.1",
		"listen '127.0.0.1' (no port) stores address correctly");
	check(serverConf.listen.at(2).second == "8000",
		"listen '8000' (no address) stores port correctly");
	check(serverConf.listen.at(3).first == "*" && serverConf.listen.at(3).second == "8000",
		"listen '*:8000' splits into correct address and port");
	check(serverConf.listen.at(4).first == "localhost" && serverConf.listen.at(4).second == "8000",
		"listen 'localhost:8000' splits into correct address and port");
}

void	ConfigParserTests::test_addServerNames()
{
	t_serverConf	serverConf;
	bool			success = true;
	bool			fail = false;

	try {
		addServerNames(serverConf, "mysite.com www.mysite.com");
	} catch (const std::exception &e) {
		success = false;
	}
	try {
		addServerNames(serverConf, "");
	} catch (const std::exception &e) {
		fail = true;
	}

	check(success, "addServerNames accepts a valid input without throwing");
	check(fail, "addServerNames throws on empty string");
	check(serverConf.serverNames.size() == 2, "addServerNames stores correct number of names");
	check(serverConf.serverNames.at(0) == "mysite.com", "addServerNames stores first name correctly");
	check(serverConf.serverNames.at(1) == "www.mysite.com", "addServerNames stores second name correctly");
}

void	ConfigParserTests::test_checkAutoindex()
{
	bool	success = true;
	bool	fail = false;
	int		onResult = -1;
	int		offResult = -1;

	try {
		onResult = checkAutoindex("on");
		offResult = checkAutoindex("off");
	} catch (const std::exception &e) {
		success = false;
	}
	try {
		checkAutoindex("garbage");
	} catch (const std::exception &e) {
		fail = true;
	}

	check(success, "checkAutoindex accepts 'on' and 'off' without throwing");
	check(fail, "checkAutoindex throws on invalid input");
	check(onResult == 1, "checkAutoindex 'on' returns 1");
	check(offResult == 0, "checkAutoindex 'off' returns 0");
}

void	ConfigParserTests::test_strToNum()
{
	int		num;
	bool	fail = false;

	try {
		num = strToNum("8080");
	} catch (const std::exception &e) {
		fail = true;
	}
	check(num == 8080, "convert a string to int successfully");
	try {
		num = strToNum("a8080");
	} catch (const std::exception &e) {
		fail = true;
	}
	check(fail, "convert string containing non-numerical characters throws error");
	try {
		num = strToNum("-8080");
	} catch (const std::exception &e) {
		fail = true;
	}
	check(fail, "convert string containing non-numerical characters throws error");
}

void	ConfigParserTests::test_split_chunk()
{
	typedef struct s_case {
		std::string	testString;
		std::string	expectedFirst;
		std::string	expectedSecond;
		std::string	testName;
	} t_case;

	t_case	testCases[] = {
		{"http ", "http", "", "splitting block directive (has space between keyword and separator)"},
		{"http", "http", "", "splitting block directive (has no space between keyword and separator)"},
		{"	    	http  	   	", "http", "", "splitting block directive (has multiple space/tabs between keyword and separator as well as front and back)"},
		{"include mime.types", "include", "mime.types", "splitting simple directive (has space between keyword and separator)"},
		{"  	include mime.types 		 		", "include", "mime.types", "splitting simple directive (has space between keyword and separator and multiple spaces front and back)"},
		{"\ninclude\nmime.types\n", "include", "mime.types", "splitting simple directive (has newline between keyword and separator and newline front and back)"},
		{"  	include			  mime.types 		 		", "include", "mime.types", "splitting simple directive (has space and tabs between keyword and separator and multiple spaces front and back)"},
		{"includemime.types  		", "includemime.types", "", "splitting wrong simple directive (has no space between keyword and separator)"},
		{"listen 8080", "listen", "8080", "splitting simple directive (has space between keyword and separator)"},
		{"location /fruits", "location", "/fruits", "splitting simple directive (has space between keyword and separator)"},
	};

	for (unsigned long i = 0; i < (sizeof(testCases) / sizeof(testCases[0])); i++)
	{
		std::pair<std::string, std::string> splt = splitter(testCases[i].testString);
		check(splt.first == testCases[i].expectedFirst && splt.second == testCases[i].expectedSecond, testCases[i].testName);
	}

	bool	fail = false;
	try {
		splitter(" \t\n");
	} catch (const std::exception &e) {
		fail = true;
	}
	check(fail, "string with only whitespace throws error");
}

void	ConfigParserTests::test_read_valid_file_correctly()
{
	std::string str;
	std::string str_neg;
	bool		success = true;
	bool		fail = false;

	try {
		readConfigToString("../config/minimal.conf", str);
	} catch (const std::exception &e) {
		success = false;
	}
	try {
		readConfigToString("non_existing_file", str_neg);
	} catch (const std::exception &e) {
		fail = true;
	}

	check(success, "reading an existing file is successful");
	check(fail, "reading a non exisiting file throws error");
	check(!str.empty(), "reading an existing file returns non-empty content");
}

void	ConfigParserTests::run_all()
{
	std::cout << "\n\033[30;105mRunning ConfigParserTests...\033[0m\n" << std::endl;

	test_read_valid_file_correctly();
	test_split_chunk();
	test_strToNum();
	test_checkAutoindex();
	test_addServerNames();
	test_addListenAddressPort();
	test_addErrorPages();
	test_addAllowedMethods();
	test_addRedirection();

	printSummary();
}