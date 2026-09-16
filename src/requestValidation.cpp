/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestValidation.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 17:39:15 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/16 17:57:59 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

std::string createRedirectPath(t_locationConf *location)
{
	std::string	path;
	//if redirect path hs no / at start
		//then just return path, otherwise append root path.
	if (location->redirection.second.find('/') == std::string::npos)
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
	std::cout << "GARBANZO"  << std::endl;
	if (location->redirection.first != 0)
	{
		std::cout << "VUALA "  << std::endl;
		responseInstructions.statusCode = location->redirection.first;
		responseInstructions.isRedirect = true;
		responseInstructions.redirectLocation = createRedirectPath(location);
		return (responseInstructions);
	}
	// if ()
	
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
