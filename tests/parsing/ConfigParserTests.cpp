/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserTests.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 11:29:09 by pecastro          #+#    #+#             */
/*   Updated: 2026/08/24 16:59:49 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParserTests.hpp"
#include "../../include/webserv.hpp"

ConfigParserTests::ConfigParserTests() : TestSuite("ConfigParserTests") {}

//add test methods here

void	ConfigParserTests::test_read_valid_file_correctly()
{
	// const char *filename = "config/minimal.conf";
	std::string str;
	std::string str_neg;
	int			ret = readConfigFileToString("../config/minimal.conf", str);
	int			ret_neg = readConfigFileToString("non_existing_file", str_neg);
	
	check(ret == 0, "reading an existing file returns success code 0");
	check(ret_neg == 1, "reading a non exisiting file returns error code 1");
	check(!str.empty(), "reading an existing file returns non-empty content");
}

void	ConfigParserTests::test_split_chunk()
{
	std::string 						str;
	std::pair<std::string, std::string>	pairDirective1 = splitter("http ");
	std::pair<std::string, std::string>	pairDirective2 = splitter("http");
	std::pair<std::string, std::string>	pairDirective3 = splitter("	    	http  	   	");
	std::pair<std::string, std::string>	pairDirective4 = splitter("include mime.types");
	std::pair<std::string, std::string>	pairDirective5 = splitter("  	include mime.types 		 		");
	//have yet to check in nginx config file if it is possible also to have tabs or other white spaces between keyword and param
	std::pair<std::string, std::string>	pairDirective6 = splitter("includemime.types  		");
	std::pair<std::string, std::string>	pairDirective7 = splitter("listen 8080");
	std::pair<std::string, std::string>	pairDirective8 = splitter("location /fruits");

	check(pairDirective1.first == "http" && pairDirective1.second == "", "splitting block directive (has space between keyword and separator)");
	check(pairDirective2.first == "http" && pairDirective2.second == "", "splitting block directive (has no space between keyword and separator)");
	check(pairDirective3.first == "http" && pairDirective3.second == "", "splitting block directive (has multiple space/tabs between keyword and separator as well as front and back)");
	check(pairDirective4.first == "include" && pairDirective4.second == "mime.types", "splitting simple directive (has space between keyword and separator)");
	check(pairDirective5.first == "include" && pairDirective5.second == "mime.types", "splitting simple directive (has space between keyword and separator and multiple spaces front and back)");
	check(pairDirective6.first == "includemime.types" && pairDirective6.second == "", "splitting wrong simple directive (has no space between keyword and separator)");
	check(pairDirective7.first == "listen" && pairDirective7.second == "8080", "splitting simple directive (has space between keyword and separator)");
	check(pairDirective8.first == "location" && pairDirective8.second == "/fruits", "splitting simple directive (has space between keyword and separator)");
}

void	ConfigParserTests::run_all()
{
	std::cout << "\n\033[30;105mRunning ConfigParserTests...\033[0m\n" << std::endl;

	test_read_valid_file_correctly();
	test_split_chunk();

	printSummary();
}