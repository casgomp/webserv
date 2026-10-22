/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestValidation.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 17:39:15 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/22 17:28:29 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <iostream>
# include <unistd.h>
# include <string>

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
	std::cout << "location == " << location->path << std::endl;
	if (location == NULL)
	{
		return (buildErrorInstructions(404, serverConf));
	}
	status = validateMethod(location->allowedMethods, httpRequest.requestLine.method);
	if (status != 0)
		return (buildErrorInstructions(status, serverConf));
	if (location->redirection.first != 0)
	{
		responseInstructions.statusCode = location->redirection.first;
		responseInstructions.isRedirect = true;
		responseInstructions.redirectLocation = createRedirectPath(location);
		return (responseInstructions);
	}
	normalizePath(httpRequest.requestLine.target, normalizedTarget);
	resolvedPath = joinedPath(location->root, normalizedTarget);
	isFile = (pathIsFile(resolvedPath));
	std::cout << "normalizedTarget = " << normalizedTarget << std::endl;
	std::cout << "resolvedPath = " << resolvedPath << std::endl;
	std::cout << "isFile = " << isFile << std::endl;
	if (!isFile)
	{
		//Yeah
		std::string yeah = resolvedPath.substr(resolvedPath.find_last_of("/"));
		std::cout << "yeah = " << yeah << std::endl;
		if (yeah == "/Yeah")
			return (buildErrorInstructions(404, serverConf));
		std::cout << "is not a fileeeeeeee!" << std::endl;
		if (!pathIsDir(resolvedPath) && httpRequest.requestLine.method != "POST")
		{
			std::cout << "!pathIsDir(resolvedPath)" << std::endl;
			return (buildErrorInstructions(404, serverConf));
		}
		else if(pathIsDir(resolvedPath) && httpRequest.requestLine.target[httpRequest.requestLine.target.size() - 1] != '/')
		{
			std::cout << "resolvedPAth:    " << resolvedPath << std::endl;
			std::cout << "httpRequest.requestLine.target[httpRequest.requestLine.target.size() - 1] != '/'" << std::endl;
			responseInstructions.statusCode = 301;
			responseInstructions.isRedirect = true;
			responseInstructions.redirectLocation = httpRequest.requestLine.target + '/';
			return (responseInstructions);
		}
	}
	// if (!location->cgiExtension.empty())
	std::string ext;
	std::map<std::string, std::string>::const_iterator it;
	size_t	dot = resolvedPath.find_last_of('.');
	if (dot != std::string::npos)
	{
		ext = resolvedPath.substr(dot + 1);
		it = location->cgiExtension.find(ext);
		if (isFile && it != location->cgiExtension.end())
		{
			// if (!isFile)
			// 	return (buildErrorInstructions(404, serverConf));
			// size_t	dot = resolvedPath.find_last_of('.');
			// if (dot == std::string::npos)
			// 	return (buildErrorInstructions(403, serverConf));
			// std::string ext = resolvedPath.substr(dot + 1);
			// std::map<std::string, std::string>::const_iterator it = location->cgiExtension.find(ext);
			// if (it != location->cgiExtension.end())
			if (access(resolvedPath.c_str(), R_OK) != 0)
				return (buildErrorInstructions(403, serverConf));
			responseInstructions.isCgi = true;
			responseInstructions.cgi.interpreter = it->second;
			responseInstructions.resolvedPath = resolvedPath;
			std::cout << "is Cgi validation: responseInstructions.cgi.interpreter: " << responseInstructions.cgi.interpreter << std::endl;
			responseInstructions.cgi.queryString = httpRequest.requestLine.queryString;
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
			{
				std::cout << "*****location->index.at(0)******" << location->index.at(0) << std::endl;
				resolvedPath = joinedPath(resolvedPath, location->index.at(0));
			}
			else if (pathIsFile(joinedPath(resolvedPath, FALLBACK_INDEX)))
				resolvedPath = joinedPath(resolvedPath, FALLBACK_INDEX);
			else if (location->autoindex)
				responseInstructions.isAutoIndex = true;
			else
				return (buildErrorInstructions(403, serverConf));
		}
		if (access(resolvedPath.c_str(), R_OK) != 0)
			return (buildErrorInstructions(403, serverConf));
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
			return (buildErrorInstructions(413, serverConf));
		if (getContentType(resolvedPath) == "application/octet-stream")
		{
			std::cout << "WRPONGYYYY!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
			if (!isFile)
				return (buildErrorInstructions(400, serverConf));
			std::cout << "WRPONG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
			return (buildErrorInstructions(415, serverConf));
		}
		if (isFile)
		{
			if (access(resolvedPath.c_str(), W_OK) != 0)
				return (buildErrorInstructions(403, serverConf));
		}
		else
		{
			if (access(resolvedPath.substr(0, resolvedPath.find_last_of('/')).c_str(), X_OK | W_OK) != 0)
				return (buildErrorInstructions(403, serverConf));
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
			return (buildErrorInstructions(403, serverConf));
		if (access(resolvedPath.substr(0, resolvedPath.find_last_of('/')).c_str(), X_OK | W_OK) != 0)
			return (buildErrorInstructions(403, serverConf));
		responseInstructions.statusCode = 204;
		return (responseInstructions);
	}
	return (responseInstructions);
}