/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 10:41:02 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/01 23:14:46 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpResponse.hpp"
#include <sstream>
#include <map>
#include <fstream>

#include <dirent.h>

#include <cstdio>

static std::map<int, std::string> reasonPhrase;
static bool	initialized = false;

static std::string	escapeHtml(const std::string &text)
{
	std::string	result;

	for (size_t i = 0; i < text.size(); i++)
	{
		if (text[i] == '&')
			result += "&amp;";
		else if (text[i] == '<')
			result += "&lt;";
		else if (text[i] == '>')
			result += "&gt;";
		else
			result += text[i];
	}
	return result;
}

static std::string	encodeUrl(const std::string &text)
{
	std::string		result;
	const char		*hex = "0123456789ABCDEF";

	for (size_t i = 0; i < text.size(); i++)
	{
		unsigned char c = text[i];

		if ((c >= 'A' && c <= 'Z')
			|| (c >= 'a' && c <= 'z')
			|| (c >= '0' && c <= '9')
			|| c == '-' || c == '_' || c == '.' || c == '~')
			result += c;
		else
		{
			result += '%';
			result += hex[c >> 4];
			result += hex[c & 15];
		}
	}
	return result;
}

static void buildAutoIndexBody(std::string &body, const t_responseInstructions &instructions)
{
	DIR	*dir = opendir(instructions.resolvedPath.c_str());
	if (!dir)
		return;
	struct dirent *entry;

	body = "<html>\n"
       "<head><title>Index of directory</title></head>\n"
       "<body>\n"
       "<h1>Index of directory</h1>\n";
	while ((entry = readdir(dir)) != NULL)
	{
		if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
			continue;
		std::string filename = entry->d_name;
		body += "<a href=\"" + encodeUrl(filename) + "\">";
		body += escapeHtml(filename);
		body += "</a><br>\n";
	}
	body += "</body>\n"
        "</html>\n";

	closedir(dir);
}

static void	buildBody(std::string &body,const t_responseInstructions &instructions)
{
	if (instructions.isAutoIndex)
	{
		// Generate HTML directory listing
		buildAutoIndexBody(body, instructions);
		// Store the generated HTML in body
		return;
	}
	std::ifstream	file(instructions.resolvedPath.c_str());
	if (!file.is_open())
		return;
	std::stringstream buffer;
	buffer << file.rdbuf();
	body = buffer.str();
}

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
	reasonPhrase[500] = "Internal Server Error";
}

static void	buildStatusLine(std::string &response, const t_responseInstructions &instructions)
{
	std::stringstream						ss;
	std::map<int, std::string>::iterator	it;
	std::string								code;

	if (!initialized)
	{
		initReasonPhrases(reasonPhrase);
		initialized = true;
	}
	ss << instructions.statusCode;
	it = reasonPhrase.find(instructions.statusCode);
	if (it != reasonPhrase.end())
		code = it->second;
	response =  "HTTP/1.1 " + ss.str() + " " + code + "\r\n";
}

static void	buildHeaders(std::string &response,
	const t_responseInstructions &instructions, const std::string &body)
{
	if (instructions.isAutoIndex)
		response += "Content-Type: text/html\r\n";
	else
		response += "Content-Type: " + instructions.contentType + "\r\n";
	std::stringstream len;
	len << body.size();
	response += "Content-Length: " + len.str() + "\r\n";
	if (instructions.isRedirect == true)
		response += "Location: " + instructions.redirectLocation + "\r\n";
	if (!instructions.closeConnection)
		response += "Connection: keep-alive\r\n";
	else
		response += "Connection: close\r\n";



}

static bool deleteFile(const std::string &path)
{
    return std::remove(path.c_str()) == 0;
}

static bool writePostBody(const std::string &path, const std::string &requestBody)
{
	std::ofstream file(path.c_str(), std::ios::binary | std::ios::trunc);
	if (!file.is_open())
		return false;
	file.write(requestBody.data(), requestBody.size());
	if (!file)
		return false;
	file.close();
	if (!file)
		return false;
	return true;
}

std::string	buildHttpResponse(const t_responseInstructions &instructions,
							const std::string &requestBody,
							const std::string &method)
{
	std::string				response;
	std::string				body;
	t_responseInstructions	result;

	result = instructions;
	if (method == "DELETE" && result.statusCode == 204)
	{
		if (!deleteFile(result.resolvedPath))
			result.statusCode = 500;
	}
	if (method == "POST" && result.statusCode == 201)
	{
		if (!writePostBody(result.resolvedPath, requestBody))
			result.statusCode = 500;
	}
	if (method == "GET" && result.statusCode == 200)
		buildBody(body, result);
	buildStatusLine(response, result);

	buildHeaders(response, result, body);

	response += "\r\n";
	response += body;

	return response;
}
