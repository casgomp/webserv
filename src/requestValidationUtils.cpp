/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestValidationUtils.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 15:59:14 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/27 22:56:34 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <algorithm>
# include <cstdio>
# include <map>
# include <sstream>
# include <string>
# include <sys/types.h>
# include <sys/stat.h>
# include <vector>
# include <unistd.h>
# include <iostream>

# include "requestValidationUtils.hpp"
# include "configConf.hpp"

std::map<std::string, std::string>	getValidMimeTypes()
{
	std::map<std::string, std::string>	mime;

	mime["html"] = "text/html";
	mime["htm"]  = "text/html";
	mime["css"]  = "text/css";
	mime["js"]   = "application/javascript";
	mime["json"] = "application/json";
	mime["txt"]  = "text/plain";
	mime["png"]  = "image/png";
	mime["jpg"]  = "image/jpeg";
	mime["jpeg"] = "image/jpeg";
	mime["gif"]  = "image/gif";
	mime["pdf"]  = "application/pdf";
	mime["mp4"]  = "video/mp4";
	return (mime);
}

std::string	getContentType(const std::string &path)
{
	std::map<std::string, std::string>				mime = getValidMimeTypes();
	std::map<std::string, std::string>::iterator	it;
	std::string										fallbackMime = "application/octet-stream";
	size_t											dot;
	std::string										ext;

	dot = path.find_last_of('.');
	if (dot == std::string::npos)
		return (fallbackMime);
	ext = path.substr(dot + 1);
	it = mime.find(ext);
	if (it != mime.end())
		return (it->second);
	return (fallbackMime);
}

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

bool decodePercentPath(const std::string &target, std::string &decodedTarget)
{
	decodedTarget.clear();

	for (size_t i = 0; i < target.size(); i++)
	{
		if (target[i] != '%')
		{
			decodedTarget += target[i];
			continue;
		}

		if (i + 2 >= target.size())
			return false;

		char hi = target[i + 1];
		char lo = target[i + 2];

		if (!std::isxdigit(static_cast<unsigned char>(hi)) ||
			!std::isxdigit(static_cast<unsigned char>(lo)))
			return false;

		int high = std::isdigit(static_cast<unsigned char>(hi))
			? hi - '0' : std::tolower(static_cast<unsigned char>(hi)) - 'a' + 10;
		int low = std::isdigit(static_cast<unsigned char>(lo))
			? lo - '0' : std::tolower(static_cast<unsigned char>(lo)) - 'a' + 10;

		char decoded = static_cast<char>(high * 16 + low);

		if (decoded == '\0')
			return false;

		decodedTarget += decoded;
		i += 2;
	}

	return true;
}

void	normalizePath(const std::string &target, std::string &normalizedTarget)
{
	std::stringstream			ss(target);
	std::vector<std::string>	vec;
	std::string					segment;

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
	return (location->redirection.second);
}

int	validateMethod(const std::vector<std::string> &allowedMethods, const std::string &requestMethod)
{
	int	status = 0;

	if (!(requestMethod == "GET" || requestMethod == "POST" || requestMethod == "DELETE"))
		return (405);
	else if (std::find(allowedMethods.begin(), allowedMethods.end(), requestMethod) == allowedMethods.end())
	{
		return (405);
	}
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
		path = locations[i].path;
		if (target.compare(0, path.size(), path) != 0)
			continue ;
		if (path.size() < target.size() && path[path.size() - 1] != '/' && target[path.size()] != '/')
			continue ;
		lenCurr = path.size();
		if (lenCurr > len)
		{
			len = lenCurr;
			location = &locations[i];
		}
	}
	return (location);
}
