/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestValidation.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 17:39:15 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/08 09:51:37 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <iostream>
# include <unistd.h>

# include "requestValidation.hpp"
# include "requestValidationUtils.hpp"
# include "httpRequestParser.hpp"
# include "configConf.hpp"

# define FALLBACK_INDEX "index.html"

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
	std::cout << "normalizedTarget = " << normalizedTarget << std::endl;
	resolvedPath = joinedPath(location->root, normalizedTarget);
	isFile = (pathIsFile(resolvedPath));
	std::cout << "isFile = " << isFile << std::endl;
	if (!isFile)
	{
		if (!pathIsDir(resolvedPath))
		{
			responseInstructions.statusCode = 404;
			return (responseInstructions);
		}
		if (httpRequest.requestLine.target[httpRequest.requestLine.target.size() - 1] != '/')
		{
			responseInstructions.statusCode = 301;
			responseInstructions.isRedirect = true;
			responseInstructions.redirectLocation = httpRequest.requestLine.target + '/';
			return (responseInstructions);
		}
	}
	if (!location->cgiExtension.empty())
	{
		if (!isFile)
		{
			responseInstructions.statusCode = 404;
			return (responseInstructions);
		}
		size_t	dot = resolvedPath.find_last_of('.');
		if (dot == std::string::npos)
		{
			responseInstructions.statusCode = 403;
			return (responseInstructions);
		}
		std::string ext = resolvedPath.substr(dot + 1);
		std::map<std::string, std::string>::const_iterator it = location->cgiExtension.find(ext);
		if (it != location->cgiExtension.end())
		{
			if (access(resolvedPath.c_str(), R_OK) != 0)
			{
				responseInstructions.statusCode = 403;
				return (responseInstructions);
			}
			responseInstructions.isCgi = true;
			responseInstructions.cgiInterpreter = it->second;
			responseInstructions.resolvedPath = resolvedPath;
			responseInstructions.queryString = httpRequest.requestLine.queryString;
			return (responseInstructions);
		}
	}
	if (httpRequest.requestLine.method == "GET")
	{
		std::cout << "*****GET request validation******" << std::endl;
		std::cout << "resolvedPath1: " << resolvedPath << std::endl;////////////////////////////////////////
		if (!isFile)
		{
			if (!location->index.empty())
				resolvedPath = joinedPath(resolvedPath, location->index.at(0));
			else if (pathIsFile(joinedPath(resolvedPath, FALLBACK_INDEX)))
				resolvedPath = joinedPath(resolvedPath, FALLBACK_INDEX);
			else if (location->autoindex)
				responseInstructions.isAutoIndex = true;
			else
			{
				responseInstructions.statusCode = 403;
				return (responseInstructions);
			}
		}
		if (access(resolvedPath.c_str(), R_OK) != 0)
		{
			responseInstructions.statusCode = 403;
			return (responseInstructions);
		}
		std::cout << "resolvedPath2: " << resolvedPath << std::endl;////////////////////////////////////////
		responseInstructions.resolvedPath = resolvedPath;
		responseInstructions.contentType = getContentType(resolvedPath);
		responseInstructions.statusCode = 200;
		return (responseInstructions);
	}
	else if (httpRequest.requestLine.method == "POST")
	{
		std::cout << "*****POST request validation******" << std::endl;
		if (httpRequest.body.size() > location->clientMaxBodySize)
		{
			responseInstructions.statusCode = 413;
			return (responseInstructions);
		}
		if (getContentType(resolvedPath) == "application/octet-stream")
		{
			responseInstructions.statusCode = 415;
			return (responseInstructions);
		}
		if (isFile)
		{
			if (access(resolvedPath.c_str(), W_OK) != 0)
			{
				responseInstructions.statusCode = 403;
				return (responseInstructions);
			}
		}
		else
		{
			if (access(resolvedPath.substr(0, resolvedPath.find_last_of('/')).c_str(), X_OK | W_OK) != 0)
			{
				responseInstructions.statusCode = 403;
				return (responseInstructions);
			}
		}
		responseInstructions.resolvedPath = resolvedPath;
		responseInstructions.contentType = getContentType(resolvedPath);
		responseInstructions.statusCode = 201;
		return (responseInstructions);
	}
	else if (httpRequest.requestLine.method == "DELETE")
	{
		std::cout << "*****DELETE request validation******" << std::endl;
		if (!isFile)
		{
			responseInstructions.statusCode = 403;
			return (responseInstructions);
		}
		if (access(resolvedPath.substr(0, resolvedPath.find_last_of('/')).c_str(), X_OK | W_OK) != 0)
		{
			responseInstructions.statusCode = 403;
			return (responseInstructions);
		}
		responseInstructions.statusCode = 204;
		return (responseInstructions);
	}
	return (responseInstructions);
}