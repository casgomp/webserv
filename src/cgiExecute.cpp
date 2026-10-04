/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiExecute.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 15:45:27 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/04 15:55:00 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <sstream>
# include <string>
# include <vector>

# include "httpRequestParserUtils.hpp"

std::vector<std::string>	buildCgiEnvp(HttpRequest &httpRequest)
{
	std::vector<std::string>	cgiEnvp;
	std::ostringstream			oss;
	std::string					method = httpRequest.requestLine.method;
	
	oss << httpRequest.expectedBodyLength;
	cgiEnvp.push_back("REQUEST_METHOD=" + method);
	if (method == "GET")
		cgiEnvp.push_back("QUERY_STRING=" + httpRequest.requestLine.queryString);
	else if (method == "POST")
	{
		cgiEnvp.push_back("CONTENT_LENGTH=" + oss.str());
		//cgiEnvp.push_back("CONTENT_TYPE=" + httpRequest.requestLine.contentType);//contentType is missing from httpRequest headers in cgi POST request.
	}
	return (cgiEnvp);
}