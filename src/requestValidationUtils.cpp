/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestValidationUtils.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 15:59:14 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/20 14:14:41 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

bool	pathIsDir(const std::string &resolvedPath)
{
	struct stat	statbuf;

	if (stat(resolvedPath.c_str(), &statbuf) == 0)
		return (S_ISDIR(statbuf.st_mode));
	return (false);
}

bool	pathIsFile(const std::string &resolvedPath)
{
	struct stat	statbuf;

	if (stat(resolvedPath.c_str(), &statbuf) == 0)//return: success=0, error=-1
		return (S_ISREG(statbuf.st_mode));//return: true, or false
	return (false);
}

std::string	joinedPath(std::string root, std::string normalizedTarget)
{
	std::string nRoot = root;
	std::string	nTarget = normalizedTarget;

	if (!root.empty() && root.at(root.size() - 1) == '/')
		nRoot = root.substr(0, root.size() - 1);
	if (!normalizedTarget.empty() && normalizedTarget.at(0) == '/')
		nTarget = normalizedTarget.substr(1);
	return (nRoot + "/" + nTarget);
}

void	normalizePath(const std::string &target, std::string &normalizedTarget)
{
	std::stringstream			ss(target);
	std::vector<std::string>	vec;
	std::string					segment;

	// std::cout << "target is = " << target << std::endl;
	while (getline(ss, segment,'/'))
	{
		if (segment.empty() || segment == ".")
			continue ;
		else if (segment == "..")
		{
			if (vec.empty())
			{
				normalizedTarget = "";
				return ;
			}
			vec.pop_back();
		}
		else
			vec.push_back(segment);
	}
	normalizedTarget = "/";
	for (size_t i = 0; i < vec.size(); i++)
	{
		normalizedTarget += vec[i];
		if (i + 1 < vec.size())
			normalizedTarget += "/";
	}
}

std::string createRedirectPath(t_locationConf *location)
{
	std::string	path;
	//if redirect path has no / at start return path, otherwise append root path.
	if (location->redirection.second[0] != '/')
		path = location->redirection.second;
	else
		path = joinedPath(location->root, location->redirection.second);
	// std::cout << "path**************" << path << std::endl;
	return (path);
}

int	validateMethod(const std::vector<std::string> &allowedMethods, const std::string &requestMethod)
{
	int	status = 0;

	if (!(requestMethod == "GET" || requestMethod == "POST" || requestMethod == "DELETE"))
		return (405);
	else if (std::find(allowedMethods.begin(), allowedMethods.end(), requestMethod) == allowedMethods.end())
		return (403);
	return(status);
}

t_locationConf*	matchLocation(std::vector<t_locationConf> &locations, const std::string &target)
{
	t_locationConf	*location = NULL;
	std::string		path;
	size_t			lenCurr;
	size_t			len;

	len = 0;
	for (size_t i = 0; i < locations.size(); i ++)
	{
		// std::cout << "****target = " << target << " ; location[i].path = " << locations[i].path << std::endl;
		path = locations[i].path;
		if (target.compare(0, path.size(), path) != 0)
			continue ;
		if (path.size() < target.size() && path[path.size() - 1] != '/' && target[path.size()] != '/')
			continue ; 
		lenCurr = path.size();
		if (lenCurr > len)
		{
			// std::cout << "**candidate location[i]path = " << locations[i].path << std::endl;
			len = lenCurr;
			location = &locations[i];
		}
	}
	// std::cout << "**location = " << location->path << std::endl;
	return (location);
}
