/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponseError.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 14:59:16 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/20 15:25:18 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpResponse.hpp"

static std::map<int, std::string> reasonPhrase;
static bool	initialized = false;

static void	initReasonPhrases(std::map<int, std::string> &reasonPhrase)
{
	reasonPhrase[200] = "OK";
	reasonPhrase[201] = "Created";
	reasonPhrase[204] = "No Content";
	reasonPhrase[301] = "Moved Permanently";
	reasonPhrase[307] = "Temporary Redirect";
	reasonPhrase[308] = "Permanent Redirect";
	reasonPhrase[400] = "Bad Request";
	reasonPhrase[403] = "Forbidden";
	reasonPhrase[404] = "Not Found";
	reasonPhrase[405] = "Method Not Allowed";
	reasonPhrase[413] = "Content Too Large";
	reasonPhrase[415] = "Unsupported Media Type";
	reasonPhrase[429] = "Too Many Requests";
	reasonPhrase[431] = "Request Header Fields Too Large";
	reasonPhrase[500] = "Internal Server Error";
	reasonPhrase[503] = "Service Unavailable";
	reasonPhrase[504] = "Gateway Timeout";
}

std::string getReasonPhrase(int statusCode)
{
    if (!initialized)
    {
        initReasonPhrases(reasonPhrase);
        initialized = true;
    }

    std::map<int, std::string>::const_iterator it;
    it = reasonPhrase.find(statusCode);

    if (it != reasonPhrase.end())
        return it->second;

    return "Unknown";
}

static std::string	buildErrorBody(int statusCode)
{
	std::ostringstream	body;
	std::string			message;

	message = getReasonPhrase(statusCode);
	body << "<html>\n"
		<< "<head><title>"  << statusCode << " " << message << "</title></head>\n"
		<< "<body>\n"
		<< "<center><h1>" << statusCode << " " << message << "</h1></center>\n"
		<< "<hr><center>HTTP Amigos/1.0.0</center>\n"
		<< "</body>\n"
		<< "</html>\n";
	return body.str();
}

std::string	buildConfiguredErrorBody(int statusCode, const std::map<int, std::string> &errorPages)
{
	std::map<int, std::string>::const_iterator it;

	it = errorPages.find(statusCode);
	if (it != errorPages.end() && !it->second.empty())
	{
		std::ifstream file(it->second.c_str(), std::ios::binary);
		if (file.is_open())
		{
			std::stringstream buffer;
			buffer << file.rdbuf();
			if (!file.bad())
				return buffer.str();
		}
	}
	return buildErrorBody(statusCode);
}
