/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configInterface.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 13:37:40 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/02 17:21:03 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

//getMimeTypes() //to be called when doing response and mapping extension (e.g. html) to get mime type (e.g. text/html)

void	addErrorPages(t_serverConf &serverConf, const std::string &input)
{
	if (input.empty())
	{
		std::cerr << "Error: Empty string" << std::endl;
		return ;
	}
	std::istringstream			ss(input);
	std::string					str;
	std::vector<std::string>	vec;
	int							num;
	std::map<int, std::string>	errorMap;
	
	while (ss >> str)
		vec.push_back(str);
	if (vec.back().at(0) != '/')
	{
		std::cerr << "Error: invalid uri (error_page)" << std::endl;
		return ;
	}
	for (size_t i = 0; i < vec.size() - 1; i ++)
	{
		num = strToNum(vec.at(i));
		if (!(num >= 400 && num < 600))
		{
			std::cerr << "Error: invalid code (error_page)" << std::endl;
			return ;
		}
		errorMap[num] = vec.back();
	}
	serverConf.errorPages = errorMap;
}

void	addListenAddressPort(t_serverConf &serverConf, const std::string &input)
{
	// listen 127.0.0.1:8000;
	// listen 127.0.0.1;
	// listen 8000;
	// listen *:8000;
	// listen localhost:8000;
	std::pair<std::string, std::string>	pair;
	size_t								found;

	if (input.empty())
	{
		std::cerr << "Error: Empty string" << std::endl;
		return ;
	}
	found = input.find(":");
	if (found != std::string::npos)
	{
		pair.first = input.substr(0, found);
		pair.second = input.substr(found + 1, input.length() - found + 1);
	}
	else if (input.find("."))
		pair.first = input;
	else
		pair.second = input;
	serverConf.listen.push_back(pair);
}

void	addServerNames(t_serverConf &serverConf, const std::string &input)
{
	if (input.empty())
	{
		std::cerr << "Error: Empty string" << std::endl;
		return ;
	}
	std::istringstream ss(input);
	std::string	name;
	while (ss >> name)
		serverConf.serverNames.push_back(name);
}

int	checkAutoindex(const std::string &autoindex)
{
	if (autoindex == "on")
		return (1);
	else if (autoindex == "off")
		return(0);
	std::cerr << "Error: invalid argument (autoindex)" << std::endl;
	return (-1);
}

int	strToNum(const std::string &str)
{
	if (str.empty())
	{
		std::cerr << "Error: Empty string" << std::endl;
		return(-1);
	}
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str.at(i)))
		{
			std::cerr << "Error: Contains non numeric character" << std::endl;
			return (-1);
		}
	}
	return (atoi(str.c_str()));
}

int	checkIfValidDir(const std::string &path)
{
	DIR *dir;
	dir = opendir(path.c_str());
	if (dir == NULL)
	{
		std::cerr << "Error: " << strerror(errno) << std::endl;
		return (1);
	}
	closedir(dir);
	if (access(path.c_str(), X_OK) == -1)
	{
		std::cerr << "Error: " << strerror(errno) << std::endl;
		return (1);
	}
	return (0);
}


//PASS ALSO AS ARGUMENT THE SERVERBLOCK.CHILDREN[i].FIRST.SECOND (THE PATH)?
t_locationConf	getLocationConfig(const t_block &locationTreeConf, const t_serverConf &serverConf)
{
	t_locationConf	locationConf;
	int				maxSize;
	int 			autoidx;

	//inherited: (root, client_max_body_size, autoindex)
	//others: (allowed_method) (and return for redirections?)

	
	(void)locationTreeConf;
	(void)serverConf;

	locationConf.root = serverConf.root;
	locationConf.clientMaxBodySize = serverConf.clientMaxBodySize;
	locationConf.autoindex = serverConf.autoindex;

	std::cout << "locationTreeConf.directives.size() = " << locationTreeConf.directives.size() << std::endl;

	for (size_t i = 0; i < locationTreeConf.directives.size(); i ++)
	{
		std::cout << "locationTreeConf for loop************directive = " << locationTreeConf.directives[i].first << std::endl;
		if (locationTreeConf.directives.at(i).first == "root")
		{
			if (checkIfValidDir(locationTreeConf.directives.at(i).second.c_str()))
				return t_locationConf();
			locationConf.root = locationTreeConf.directives.at(i).second;
			std::cout << "locationConf.root = " << locationConf.root << std::endl;
		}
		if (locationTreeConf.directives.at(i).first == "client_max_body_size")
		{
			maxSize = strToNum(locationTreeConf.directives.at(i).second);
			if (maxSize == -1)
				return t_locationConf();
			locationConf.clientMaxBodySize = maxSize;
			std::cout << "client_max_body_size = " << locationConf.clientMaxBodySize << std::endl;
		}
		if (locationTreeConf.directives.at(i).first == "autoindex")
		{
			autoidx = checkAutoindex(locationTreeConf.directives.at(i).second);
			if (autoidx == -1)
				return t_locationConf();
			locationConf.autoindex = autoidx;
			std::cout << "autoindex = " << locationConf.autoindex << std::endl;
		}







	}
	
	return (locationConf);
}

t_serverConf	getServerConfig(const t_block &serverTreeConf, const t_httpConf &httpConf)
{
	t_serverConf															serverConf;
	int																		maxSize;
	int 																	autoidx;
	std::vector<std::pair<std::pair<std::string, std::string>, t_block> >	locationTree;
	t_locationConf															locationConf;

	//inherited: (root, client_max_body_size, autoindex)
	//others: (server_name, listen, error_page)
	
	serverConf.root = httpConf.root;
	serverConf.clientMaxBodySize = httpConf.clientMaxBodySize;
	serverConf.autoindex = httpConf.autoindex;
	// std::cout << "serverConf.root = " << serverConf.root << std::endl;
	// std::cout << "serverConf.clientMaxBodySize = " << serverConf.clientMaxBodySize << std::endl;
	// std::cout << "serverConf.autoindex = " << serverConf.autoindex << std::endl;
	for (size_t i = 0; i < serverTreeConf.directives.size(); i ++)
	{
		std::cout << "serverTreeConf for loop************directive = " << serverTreeConf.directives[i].first << std::endl;
		if (serverTreeConf.directives.at(i).first == "root")
		{
			if (checkIfValidDir(serverTreeConf.directives.at(i).second.c_str()))
				return t_serverConf();
			serverConf.root = serverTreeConf.directives.at(i).second;
			std::cout << "serverConf.root = " << serverConf.root << std::endl;
		}
		if (serverTreeConf.directives.at(i).first == "client_max_body_size")
		{
			maxSize = strToNum(serverTreeConf.directives.at(i).second);
			if (maxSize == -1)
				return t_serverConf();
			serverConf.clientMaxBodySize = maxSize;
			std::cout << "client_max_body_size = " << serverConf.clientMaxBodySize << std::endl;
		}
		if (serverTreeConf.directives.at(i).first == "autoindex")
		{
			autoidx = checkAutoindex(serverTreeConf.directives.at(i).second);
			if (autoidx == -1)
				return t_serverConf();
			serverConf.autoindex = autoidx;
			std::cout << "autoindex = " << serverConf.autoindex << std::endl;
		}
		if (serverTreeConf.directives.at(i).first == "server_name")
		{
			addServerNames(serverConf, serverTreeConf.directives.at(i).second);
			if (serverConf.serverNames.empty())
				return t_serverConf();
			for (size_t i = 0; i < serverConf.serverNames.size(); i++)
					std::cout << "server_name " << i << " = " << serverConf.serverNames.at(i) << std::endl;
		}
		if (serverTreeConf.directives.at(i).first == "listen")
		{
			addListenAddressPort(serverConf, serverTreeConf.directives.at(i).second);
			if (serverConf.listen.empty())
			{
				std::cout << "serverConf.listen was returned empty from the parser*****" << std::endl;
				return t_serverConf();
			}
			for (size_t i = 0; i < serverConf.listen.size(); i++)
					std::cout << "portpair(listen) " << i << " = " << serverConf.listen.at(i).first << ":" << serverConf.listen.at(i).second << std::endl;
		}
		if (serverTreeConf.directives.at(i).first == "error_page")
		{
			std::cout << "error_page = " << serverTreeConf.directives.at(i).first << " = " << serverTreeConf.directives.at(i).second << std::endl;
			addErrorPages(serverConf, serverTreeConf.directives.at(i).second);
			if (serverConf.errorPages.empty())
				return t_serverConf();
			// std::map<int, std::string>::iterator it;
			// for (it = serverConf.errorPages.begin(); it != serverConf.errorPages.end(); it++)
			// {
			// 	std::cout << it->first    // string (key)
			// 			<< ':'
			// 			<< it->second   // string's value 
			// 			<< std::endl;
			// }	
		}
	}
	if (serverConf.listen.empty())
	{
		std::cout << "the listen directive was not in the config file (and it has to be)" << std::endl;
		return t_serverConf();
	}
	locationTree = serverTreeConf.children;
	if (locationTree.empty())
	{
		std::cerr << "Error: Config file must contain location block" << std::endl;
		return t_serverConf();
	}
	for (size_t i = 0; i < locationTree.size(); i ++)
	{
		std::cout << "at server final loop -> locationTree[i].first.first = " << locationTree.at(i).first.first << std::endl;
		if (locationTree.at(i).first.first != "location")
		{
			std::cerr << "Error: Invalid block directive name (location)" << std::endl;
			return t_serverConf();
		}
		else if (locationTree.at(i).first.second.at(0) != '/')
		{
			std::cerr << "Error: Invalid block directive argument (location)" << std::endl;
			return t_serverConf();
		}
		std::cout << "going to getLocationConfig" << std::endl;
		locationConf = getLocationConfig(locationTree.at(i).second, serverConf);
		if (locationConf.level.empty())
			return t_serverConf();
		serverConf.locations.push_back(locationConf);
	}
	return (serverConf);
}

t_httpConf	getConfigInterface(const t_block &pTreeConf)
{
	t_httpConf																httpConf;
	int 																	maxSize;
	std::pair<std::pair<std::string, std::string>, t_block>					httpTree;
	int 																	autoidx;
	bool																	hasRoot = false;
	bool																	hasClientMaxBodySize = false;
	bool																	hasAutoindex = false;
	std::vector<std::pair<std::pair<std::string, std::string>, t_block> >	serverTree;
	t_serverConf															serverConf;

	//std::cout << "hi in getConfigInterface************" << std::endl;
	
	if (pTreeConf.children.empty())
	{
		std::cerr << "Error: Config file must contain http block" << std::endl;
		return t_httpConf();
	}
	httpTree = pTreeConf.children[0];
	if (httpTree.first.first != "http")
	{
		std::cerr << "Error: Config file must contain http block (type)" << std::endl;
		return t_httpConf();
	}
	httpConf.level = httpTree.first.first;
	//std::cout << "httpTree.second.directives.size() = " << httpTree.second.directives.size() << std::endl;

	for (size_t i = 0; i < httpTree.second.directives.size(); i ++)
	{
		std::cout << "httpTree for loop************" << std::endl;
		if (httpTree.second.directives.at(i).first == "root")
		{
			if (checkIfValidDir(httpTree.second.directives.at(i).second.c_str()))
				return t_httpConf();
			httpConf.root = httpTree.second.directives.at(i).second;
			std::cout << "httpConf.root = " << httpConf.root << std::endl;
			hasRoot = true;
		}
		if (httpTree.second.directives.at(i).first == "client_max_body_size")
		{
			maxSize = strToNum(httpTree.second.directives.at(i).second);
			if (maxSize == -1)
				return t_httpConf();
			httpConf.clientMaxBodySize = maxSize;
			std::cout << "client_max_body_size = " << httpConf.clientMaxBodySize << std::endl;
			hasClientMaxBodySize = true;
		}
		if (httpTree.second.directives.at(i).first == "autoindex")
		{
			autoidx = checkAutoindex(httpTree.second.directives.at(i).second);
			if (autoidx == -1)
				return t_httpConf();
			httpConf.autoindex = autoidx;
			std::cout << "autoindex = " << httpConf.autoindex << std::endl;
			hasAutoindex = true;
		}
	}
	if (!hasRoot)
		httpConf.root = FALLBACK_ROOT;
	if (!hasClientMaxBodySize)
		httpConf.clientMaxBodySize = FALLBACK_CLIENT_MAX_BODY_SIZE;
	if (!hasAutoindex)
		httpConf.autoindex = FALLBACK_AUTOINDEX;
	serverTree = httpTree.second.children;
	if (serverTree.empty())
	{
		std::cerr << "Error: Config file must contain server block" << std::endl;
		return t_httpConf();
	}
	for (size_t i = 0; i < serverTree.size(); i ++)
	{
		std::cout << "at http final loop -> serverTree[i].first.first = " << serverTree.at(i).first.first << std::endl;
		if (serverTree.at(i).first.first != "server")
		{
			std::cerr << "Error: Invalid block directive name (server)" << std::endl;
			return t_httpConf();
		}
		serverConf = getServerConfig(serverTree.at(i).second, httpConf);
		if (serverConf.level.empty())
			return t_httpConf();
		httpConf.servers.push_back(serverConf);
	}
	return (httpConf);
}