/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestValidation.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 17:39:15 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/18 15:26:20 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

bool	targetIsFile(std::string &resolvedPath)
{
	std::string	strpath = resolvedPath;
	const char	*path = strpath.c_str();
	struct stat statbuf;
	bool 		isfile = 0;

	if (stat(path, &statbuf) == 0)
		isfile = S_ISREG(statbuf.st_mode);
	return (!isfile);
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
		path = location->root + location->redirection.second;
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
	size_t			start;
	size_t			lenCurr;
	size_t			len;

	len = 0;
	for (size_t i = 0; i < locations.size(); i ++)
	{
		std::cout << "****target = " << target << " ; location[i].path = " << locations[i].path << std::endl;
		start = target.find(locations[i].path);
		if (start == 0)
		{
			lenCurr = locations[i].path.size();
			if (lenCurr > len)
			{
				std::cout << "**candidate location[i]path = " << locations[i].path << std::endl;
				len = lenCurr;
				location = &locations[i];
			}
		}
	}
	std::cout << "**location = " << location->path << std::endl;
	return (location);
}

t_responseInstructions requestValidation(HttpRequest &httpRequest, t_serverConf *serverConf)
{
	t_responseInstructions	responseInstructions;
	t_locationConf 			*location;
	int						status;
	std::string				normalizedTarget;
	std::string				resolvedPath;
	bool					isFile = 0;

	location = matchLocation(serverConf->locations, httpRequest.requestLine.target);
	if (location == NULL)
	{
		responseInstructions.statusCode = 404;
		return (responseInstructions);
	}
	status = validateMethod(location->allowedMethods, httpRequest.requestLine.method);
	if (status != 0)
	{
		responseInstructions.statusCode = status;
		return (responseInstructions);
	}
	if (location->redirection.first != 0)
	{
		responseInstructions.statusCode = location->redirection.first;
		responseInstructions.isRedirect = true;
		responseInstructions.redirectLocation = createRedirectPath(location);
		return (responseInstructions);
	}
	normalizePath(httpRequest.requestLine.target, normalizedTarget);
	// std::cout << "normalizedTarget = " << normalizedTarget << std::endl;
	resolvedPath = joinedPath(location->root, normalizedTarget);
	isFile = (targetIsFile(resolvedPath) == 0);
	if (isFile)
		responseInstructions.resolvedPath = resolvedPath;
	else if (httpRequest.requestLine.target[httpRequest.requestLine.target.size() - 1] != '/')
	{
		responseInstructions.statusCode = 301;
		responseInstructions.isRedirect = true;
		responseInstructions.redirectLocation = httpRequest.requestLine.target + '/';
		return (responseInstructions);
	}
	if (location->isCgi)
	{
		if (!isFile)
		{
			responseInstructions.statusCode = 404;
			return (responseInstructions);
		}
		if (access(resolvedPath.c_str(), X_OK) != 0)
		{
			responseInstructions.statusCode = 403;
			return (responseInstructions);
		}
		responseInstructions.isCgi = true;
		return(responseInstructions);
	}
	//change to switch case?
	if (httpRequest.requestLine.method == "GET")
	{
		if (!isFile)
		{
			//if there's a index.html file
			//if targetIsFile(resolvePath + "/index.html") ...and rename function to isFile

			//else if index

			//else if autoindex

		}
		//functionvalidate file read permissions
	}
	else if (httpRequest.requestLine.method == "POST")
	{
		//functionvalidate file permissions
	}
	else if (httpRequest.requestLine.method == "DELETE")
	{
		//functionvalidate ? file permissions
	}
	return (responseInstructions);
}


//VALIDATE (in the following order):
//1. route-path matching for /fruits, at parsing, even if url contains fruitsaaaa, it's correct. So has
//to be something like fruitas, so not matching the full word. Example: target is /docs/docs and I have locations 
//docs and /docs/docs/ ....both match but docs/docs/ wins because it is longer. Find the location at beginning of the target.
	//404 (Not found)
//2. allowed methods 
	//400 (Bad request): parsing finds invalid char such as lowercase//Erjon parser
	//405 (Method not allowed): no invalid chars, but method does not exist (can also be handled in parsing)
	//403 (Forbidden): when no parsing errors and methods exits, but is not allowed.
//3. return(redirection) status is specified in the return directive
//if redirection path in config has trailing '/' then send root+path, otherwise just path.
	//306 (Temporary Redirect)
	//307 (Permanent Redirect)
//4. check if target path ends in file then use stat() to check if file exists;
//5. check if target path has no trailing '/'
	//301 (Moved permanently)...must send the path with / at end, where the resource is.
//6. What happens if path contains only directory, so no specific file:
	//default is index.html (i.e. that's the default index even before http level which location will inherit if it isn't overriden first)
	//index...can specify index.html, or something else like fruits.html or any file type. if none of the files in index is found, then:
	//autoindex ...if autoindex is on, then send a little html display with menu at current locatin (i.e. what bash ls does), else:
	//403 (Forbidden)....404 would seem more natural, but it's a matter of security not revealing what exists on that dir (the dir is already correct).
//7. POST - There's no standard Nginx behavior so we'll implement ours in the following order:
	//413 (Content too large) i.e. compare body size against client_max_body_size
	//415 (Unsuported media type) i.e. compare file.type in request path, against types in our container with
	//supported mime types. Don't compare against content-type in the request header.
	//Check if upload (or whatever name) has permissions and create a file inside and copy body contents:
	//201 (Created)
//8. DELETE
	//204 (No content)
//9. path security and permissions:
	//allows ../ but only until reaching root
	//check permissions with opendir and access or status

//EXTRAS
	//429 Too many requests
	//idle client timeout.