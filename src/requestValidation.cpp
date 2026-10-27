/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestValidation.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 17:39:15 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/27 23:01:45 by erjonbara        ###   ########.fr       */
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
	std::string				decodedTarget;
	std::string				resolvedPath;
	bool					isFile = 0;

	if (!decodePercentPath(httpRequest.requestLine.target, decodedTarget))
		return (buildErrorInstructions(400, serverConf));

	normalizePath(decodedTarget, normalizedTarget);
	if (normalizedTarget.empty())
		return (buildErrorInstructions(403, serverConf));
	location = matchLocation(serverConf->locations, decodedTarget);
	if (location == NULL)
		return (buildErrorInstructions(404, serverConf));
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
	resolvedPath = joinedPath(location->root, normalizedTarget);
	isFile = (pathIsFile(resolvedPath));
	if (isFile && httpRequest.requestLine.target[httpRequest.requestLine.target.size() - 1] == '/')
    	return (buildErrorInstructions(404, serverConf));
	if (!isFile)
	{
		if (!pathIsDir(resolvedPath) && httpRequest.requestLine.method != "POST")
			return (buildErrorInstructions(404, serverConf));
		else if (pathIsDir(resolvedPath) &&
					httpRequest.requestLine.target[httpRequest.requestLine.target.size() - 1] != '/')
		{
			responseInstructions.statusCode = 301;
			responseInstructions.isRedirect = true;
			responseInstructions.redirectLocation = normalizedTarget + '/';
			return (responseInstructions);
		}
	}
	std::string ext;
	std::map<std::string, std::string>::const_iterator it;
	size_t	dot = resolvedPath.find_last_of('.');
	if (dot != std::string::npos)
	{
		ext = resolvedPath.substr(dot + 1);
		it = location->cgiExtension.find(ext);
		if ((isFile ||
				(!isFile && httpRequest.requestLine.method == "POST"))
				&& it != location->cgiExtension.end())
		{
			if (isFile && access(resolvedPath.c_str(), R_OK) != 0)
				return (buildErrorInstructions(403, serverConf));
			if (httpRequest.body.size() > location->clientMaxBodySize)
				return (buildErrorInstructions(413, serverConf));
			responseInstructions.isCgi = true;
			responseInstructions.cgi.interpreter = it->second;
			responseInstructions.resolvedPath = resolvedPath;
			responseInstructions.cgi.queryString = httpRequest.requestLine.queryString;
			return (responseInstructions);
		}
		else if(!isFile && it != location->cgiExtension.end()
					&& httpRequest.requestLine.method != "POST")
			return (buildErrorInstructions(404, serverConf));
	}
	if (httpRequest.requestLine.method == "GET")
	{
		if (!isFile)
		{
			bool indexFound = false;
			for (size_t i = 0; i < location->index.size(); i++)
			{
				std::string candidate = joinedPath(resolvedPath, location->index[i]);
				if (pathIsFile(candidate))
				{
					resolvedPath = candidate;
					indexFound = true;
					break ;
				}
			}
			if (!indexFound)
			{
				if (pathIsFile(joinedPath(resolvedPath, FALLBACK_INDEX)))
					resolvedPath = joinedPath(resolvedPath, FALLBACK_INDEX);
				else if (location->autoindex)
					responseInstructions.isAutoIndex = true;
				else
					return  (buildErrorInstructions(403, serverConf));
			}
		}
		if (access(resolvedPath.c_str(), F_OK) != 0)
				return (buildErrorInstructions(404, serverConf));
		if (access(resolvedPath.c_str(), R_OK) != 0)
			return (buildErrorInstructions(403, serverConf));
		responseInstructions.resolvedPath = resolvedPath;
		responseInstructions.contentType = getContentType(resolvedPath);
		responseInstructions.statusCode = 200;
		return (responseInstructions);
	}
	else if (httpRequest.requestLine.method == "POST")
	{
		if (httpRequest.body.size() > location->clientMaxBodySize)
			return (buildErrorInstructions(413, serverConf));
		if (getContentType(resolvedPath) == "application/octet-stream")
		{
			if (!isFile)
			{
				responseInstructions.statusCode = 200;
				return (responseInstructions);
			}
			return (buildErrorInstructions(415, serverConf));
		}
		if (isFile)
		{
			if (access(resolvedPath.c_str(), W_OK) != 0)
				return (buildErrorInstructions(403, serverConf));
		}
		else
		{
			std::string parent = resolvedPath.substr(0, resolvedPath.find_last_of('/'));

			if (!pathIsDir(parent))
				return (buildErrorInstructions(404, serverConf));
			if (access(parent.c_str(), X_OK | W_OK) != 0)
				return (buildErrorInstructions(403, serverConf));
		}
		responseInstructions.resolvedPath = resolvedPath;
		responseInstructions.contentType = getContentType(resolvedPath);
		responseInstructions.statusCode = 201;
		return (responseInstructions);
	}
	else if (httpRequest.requestLine.method == "DELETE")
	{
		std::cout << "before respomse path = " << resolvedPath << std::endl;
		if (!isFile)
			return (buildErrorInstructions(403, serverConf));
		if (access(resolvedPath.substr(0, resolvedPath.find_last_of('/')).c_str(), X_OK | W_OK) != 0)
			return (buildErrorInstructions(403, serverConf));
		responseInstructions.statusCode = 204;
		responseInstructions.resolvedPath = resolvedPath;
		return (responseInstructions);
	}
	return (responseInstructions);
}