/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserTests.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 11:29:09 by pecastro          #+#    #+#             */
/*   Updated: 2026/08/31 13:35:33 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParserTests.hpp"
#include "../../include/webserv.hpp"

ConfigParserTests::ConfigParserTests() : TestSuite("ConfigParserTests") {}

//add test methods here

void	ConfigParserTests::test_read_valid_file_correctly()
{
	std::string str;
	std::string str_neg;
	int			ret = readConfigToString("../config/minimal.conf", str);
	int			ret_neg = readConfigToString("non_existing_file", str_neg);
	
	check(ret == 0, "reading an existing file returns success code 0");
	check(ret_neg == 1, "reading a non exisiting file returns error code 1");
	check(!str.empty(), "reading an existing file returns non-empty content");
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
}

void	ConfigParserTests::run_all()
{
	std::cout << "\n\033[30;105mRunning ConfigParserTests...\033[0m\n" << std::endl;

	test_read_valid_file_correctly();
	test_split_chunk();

	printSummary();
}