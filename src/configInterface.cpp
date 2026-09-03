/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configInterface.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 13:37:40 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/03 18:39:03 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

//getMimeTypes() //to be called when doing response and mapping extension (e.g. html) to get mime type (e.g. text/html)
//checking if *:port, to be done at server init.

t_locationConf	getLocationConfig(const t_block &locationTreeConf, const t_serverConf &serverConf)
{
	t_locationConf	locationConf;

	locationConf.root = serverConf.root;
	locationConf.clientMaxBodySize = serverConf.clientMaxBodySize;
	locationConf.autoindex = serverConf.autoindex;

	for (size_t i = 0; i < locationTreeConf.directives.size(); i ++)
	{
		std::cout << "locationTreeConf for loop************directive = " << locationTreeConf.directives[i].first << std::endl;
		if (locationTreeConf.directives.at(i).first == "root")
		{
			checkIfValidDir(locationTreeConf.directives.at(i).second.c_str());
			locationConf.root = locationTreeConf.directives.at(i).second;
		}
		if (locationTreeConf.directives.at(i).first == "client_max_body_size")
			locationConf.clientMaxBodySize = strToNum(locationTreeConf.directives.at(i).second);
		if (locationTreeConf.directives.at(i).first == "autoindex")
			locationConf.autoindex = checkAutoindex(locationTreeConf.directives.at(i).second);
		if (locationTreeConf.directives.at(i).first == "allowed_methods")
			addAllowedMethods(locationConf, locationTreeConf.directives.at(i).second);
		if (locationTreeConf.directives.at(i).first == "return")
			addRedirection(locationConf, locationTreeConf.directives.at(i).second);
	}
	return (locationConf);
}

t_serverConf	getServerConfig(const t_block &serverTreeConf, const t_httpConf &httpConf)
{
	t_serverConf															serverConf;
	bool																	hasListen = false;
	std::vector<std::pair<std::pair<std::string, std::string>, t_block> >	locationTree;
	t_locationConf															locationConf;
	
	serverConf.root = httpConf.root;
	serverConf.clientMaxBodySize = httpConf.clientMaxBodySize;
	serverConf.autoindex = httpConf.autoindex;
	for (size_t i = 0; i < serverTreeConf.directives.size(); i ++)
	{
		std::cout << "serverTreeConf for loop************directive = " << serverTreeConf.directives[i].first << std::endl;
		if (serverTreeConf.directives.at(i).first == "root")
		{
			checkIfValidDir(serverTreeConf.directives.at(i).second.c_str());
			serverConf.root = serverTreeConf.directives.at(i).second;
		}
		if (serverTreeConf.directives.at(i).first == "client_max_body_size")
			serverConf.clientMaxBodySize = strToNum(serverTreeConf.directives.at(i).second);
		if (serverTreeConf.directives.at(i).first == "autoindex")
			serverConf.autoindex = checkAutoindex(serverTreeConf.directives.at(i).second);
		if (serverTreeConf.directives.at(i).first == "server_name")
			addServerNames(serverConf, serverTreeConf.directives.at(i).second);
		if (serverTreeConf.directives.at(i).first == "listen")
		{
			addListenAddressPort(serverConf, serverTreeConf.directives.at(i).second);
			hasListen = true;
		}
		if (serverTreeConf.directives.at(i).first == "error_page")
			addErrorPages(serverConf, serverTreeConf.directives.at(i).second);
	}
	if (hasListen == false)
		throw std::runtime_error("config file does not contain listen directive");
	locationTree = serverTreeConf.children;
	if (locationTree.empty())
		throw std::runtime_error("config file does not contain location block");
	for (size_t i = 0; i < locationTree.size(); i ++)
	{
		if (locationTree.at(i).first.first != "location")
			throw std::runtime_error("invalid block directive name (location)");
		if (locationTree.at(i).first.second.at(0) != '/')
			throw std::runtime_error("invalid block directive argument (location)");
		locationConf = getLocationConfig(locationTree.at(i).second, serverConf);
		locationConf.path = locationTree.at(i).first.second;
		serverConf.locations.push_back(locationConf);
	}
	return (serverConf);
}

t_httpConf	getConfigInterface(const t_block &pTreeConf)
{
	t_httpConf																httpConf;
	std::pair<std::pair<std::string, std::string>, t_block>					httpTree;
	bool																	hasRoot = false;
	bool																	hasClientMaxBodySize = false;
	bool																	hasAutoindex = false;
	std::vector<std::pair<std::pair<std::string, std::string>, t_block> >	serverTree;
	t_serverConf															serverConf;
	
	if (pTreeConf.children.empty())
		throw std::runtime_error("does not contain block (http)");
	httpTree = pTreeConf.children[0];
	if (httpTree.first.first != "http")
		throw std::runtime_error("invalid block directive name (http)");
	for (size_t i = 0; i < httpTree.second.directives.size(); i ++)
	{
		std::cout << "httpTree for loop************" << std::endl;
		if (httpTree.second.directives.at(i).first == "root")
		{
			checkIfValidDir(httpTree.second.directives.at(i).second.c_str());
			httpConf.root = httpTree.second.directives.at(i).second;
			hasRoot = true;
		}
		if (httpTree.second.directives.at(i).first == "client_max_body_size")
		{
			httpConf.clientMaxBodySize = strToNum(httpTree.second.directives.at(i).second);
			hasClientMaxBodySize = true;
		}
		if (httpTree.second.directives.at(i).first == "autoindex")
		{
			httpConf.autoindex = checkAutoindex(httpTree.second.directives.at(i).second);
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
		throw std::runtime_error("does not contain block (server)");
	for (size_t i = 0; i < serverTree.size(); i ++)
	{
		if (serverTree.at(i).first.first != "server")
			throw std::runtime_error("invalid block directive name (server)");
		serverConf = getServerConfig(serverTree.at(i).second, httpConf);
		httpConf.servers.push_back(serverConf);
	}
	return (httpConf);
}