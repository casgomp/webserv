/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestValidation.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 17:39:15 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/14 18:19:14 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

t_responseInstructions requestValidation(HttpRequest &httpRequest, t_serverConf *serverConf)
{
	;
}

//use responseInstructions struct to create the actual response string for clients[fd].response.

//VALIDATE (in the following order):
//0. check for status errors...e.g. such as 500 from requestRouting so already send error message.
//1. route-path matching for /fruits, at parsing, even if url contains fruitsaaaa, it's correct. So has
//to be something like fruitas, so not matching the full word.
	//404 (Not found)
	//check file permissions as well?
//2. allowed methods 
	//400 (Bad request): parsing finds invalid char such as lowercase
	//405 (Method not allowed): no invalid chars, but method does not exist (can also be handled in parsing)
	//403 (Forbidden): when no parsing errors and methods exits, but is not allowed.
//3. return(redirection) status is specified in the return directive:
	//306 (Temporary Redirect)
	//307 (Permanent Redirect)
//4. check if path has no trailing '/'
	//301 (Moved permanently)...must send the full path with / at end, where the resource is.
//5. What happens if path contains only directory, so no specific file:
	//default is index.html (i.e. that's the default index even before http level which location will inherit if it isn't overriden first)
	//index...can specify index.html, or something else like fruits.html or any file type. if none of the files in index is found, then:
	//autoindex ...if autoindex is on, then send a little html display with menu at current locatin (i.e. what bash ls does), else:
	//403 (Forbidden)....404 would seem more natural, but it's a matter of security not revealing what exists on that dir (the dir is already correct).
//6. POST - There's no standard Nginx behavior so we'll implement ours in the following order:
	//413 (Content too large) i.e. compare body size against client_max_body_size
	//415 (Unsuported media type) i.e. compare file.type in request path, against types in our container with
	//supported mime types. Don't compare against content-type in the request header.
	//Check if upload (or whatever name) has permissions and create a file inside and copy body contents:
	//201 (Created)
//7. DELETE
	//204 (No content)
//8. path security and permissions:
	//allows ../ but only until reaching root
	//check permissins with opendir and access or status


//EXTRAS
	//429 Too many requests.
