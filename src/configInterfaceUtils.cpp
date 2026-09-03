/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configInterfaceUtils.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 18:16:37 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/03 18:39:16 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

void	addRedirection(t_locationConf &locationConf, const std::string &input)
{
	if (input.empty())
		throw std::runtime_error("empty string (redirection)");
	
	std::istringstream			ss(input);
	std::string					str;
	std::vector<std::string>	vec;
	int							num;

	while (ss >> str)
		vec.push_back(str);
	if (vec.size() < 2)
		throw std::runtime_error("arguments < 2 (redirection)");
	if (vec.back().at(0) != '/')
		throw std::runtime_error("invalid uri (redirection)");
	num = strToNum(vec.at(0));
	if (!(num >= 300 && num < 400))
			throw std::runtime_error("invalid code (redirection)");
	locationConf.redirection = make_pair(num, vec.back());
}

void	addAllowedMethods(t_locationConf &locationConf, const std::string &input)
{
	if (input.empty())
		throw std::runtime_error("empty string (allowed_methods)");
	
	std::istringstream						ss(input);
	std::string								str;
	std::vector<std::string>				vec;

	while (ss >> str)
	{
		if (!(str == "GET" || str == "POST" || str == "DELETE"))
			throw std::runtime_error("invalid method");
		vec.push_back(str);
	}
	locationConf.allowedMethods = vec;
}

void	addErrorPages(t_serverConf &serverConf, const std::string &input)
{
	if (input.empty())
		throw std::runtime_error("empty string (error_page)");
		
	std::istringstream			ss(input);
	std::string					str;
	std::vector<std::string>	vec;
	int							num;
	std::map<int, std::string>	errorPageMap;
	
	while (ss >> str)
		vec.push_back(str);
	if (vec.size() < 2)
		throw std::runtime_error("arguments < 2 (redirection)");
	if (vec.back().at(0) != '/')
		throw std::runtime_error("invalid uri (error_page)");
	for (size_t i = 0; i < vec.size() - 1; i ++)
	{
		num = strToNum(vec.at(i));
		if (!(num >= 400 && num < 600))
			throw std::runtime_error("invalid code (error_page)");
		errorPageMap[num] = vec.back();
	}
	serverConf.errorPages = errorPageMap;
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
		throw std::runtime_error("empty string (listen)");
	found = input.find(":");
	if (found != std::string::npos)
	{
		pair.first = input.substr(0, found);
		pair.second = input.substr(found + 1, input.length() - found + 1);
	}
	else if (input.find(".") != std::string::npos)
		pair.first = input;
	else
		pair.second = input;
	serverConf.listen.push_back(pair);
}

void	addServerNames(t_serverConf &serverConf, const std::string &input)
{
	if (input.empty())
		throw std::runtime_error("empty string (server_name)");
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
	throw std::runtime_error("invalid argument (autoindex)");
}

int	strToNum(const std::string &str)
{
	if (str.empty())
		throw std::runtime_error("empty string for int conversion");
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str.at(i)))
			throw std::runtime_error("string contains non numeric character");
	}
	return (atoi(str.c_str()));
}

void	checkIfValidDir(const std::string &path)
{
	DIR *dir;
	dir = opendir(path.c_str());
	if (dir == NULL)
		throw std::runtime_error(strerror(errno));
	closedir(dir);
	if (access(path.c_str(), X_OK) == -1)
		throw std::runtime_error(strerror(errno));
}
