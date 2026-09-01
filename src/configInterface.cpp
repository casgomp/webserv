/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configInterface.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 13:37:40 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/01 19:05:01 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

//getMimeTypes() //to be called when doing response and mapping extension (e.g. html) to get mime type (e.g. text/html)		

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
		std::cerr << "Error: String is empty" << std::endl;
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
		std::cerr << "Error opendir: " << strerror(errno) << std::endl;
		return (1);
	}
	closedir(dir);
	if (access(path.c_str(), X_OK) == -1)
	{
		std::cerr << "Error x: " << strerror(errno) << std::endl;
		return (1);
	}
	return (0);
}

t_locationConf	getLocationConfig(const t_block &locationTreeConf)
{
	t_locationConf	locationConf;

	(void)locationTreeConf;
	//location level:
		//parse directives
			//inherited: (root, client_max_body_size, autoindex)
			//others: (allowed_method)
	return (locationConf);
}

t_serverConf	getServerConfig(const t_block &serverTreeConf, const t_httpConf &httpConf)
{
	t_serverConf	serverConf;
	int				maxSize;
	int 			autoidx;

	//for loop directives...start with inherited and use functions
	//inherited: (root, client_max_body_size, autoindex)
	//others: (server_name, listen, error_page)
	serverConf.root = httpConf.root;
	serverConf.clientMaxBodySize = httpConf.clientMaxBodySize;
	serverConf.autoindex = httpConf.autoindex;
	std::cout << serverConf.root << std::endl;
	std::cout << serverConf.clientMaxBodySize << std::endl;
	std::cout << serverConf.autoindex << std::endl;
	for (size_t i = 0; i < serverTreeConf.directives.size(); i ++)
	{
		std::cout << "serverTreeConf for loop************" << std::endl;
		if (serverTreeConf.directives.at(i).first == "root")
		{
			if (checkIfValidDir(serverTreeConf.directives.at(i).second.c_str()))
				return t_serverConf();
			serverConf.root = serverTreeConf.directives.at(i).second;
			std::cout << "httpConf.root = " << serverConf.root << std::endl;
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
	}



	//check if there are children ...loop children calling getLocationConfig
	


	//for every location call the location function with its block...
	//for location must check both children.first.first, and children.first.second
	//check validity and 
	//push to serverConf.locations.push_back();
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
		if (serverTree.at(i).first.first == "server")
		{
			serverConf = getServerConfig(serverTree.at(i).second, httpConf);
			if (serverConf.level.empty())
				return t_httpConf();
			httpConf.servers.push_back(serverConf);
		}
	}
	return (httpConf);
}