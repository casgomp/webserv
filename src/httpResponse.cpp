/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebara <ebara@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 10:41:02 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/26 16:36:08 by ebara            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpResponse.hpp"

#include <dirent.h>
#include <cstdio>

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

static bool buildAutoIndexBody(std::string &body, const t_responseInstructions &instructions)
{
	DIR	*dir = opendir(instructions.resolvedPath.c_str());
	if (!dir)
		return false;
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
	return true;
}

static bool	buildBody(std::string &body,const t_responseInstructions &instructions)
{
	if (instructions.isAutoIndex)
		return buildAutoIndexBody(body, instructions);

	std::ifstream	file(instructions.resolvedPath.c_str(), std::ios::binary);
	if (!file.is_open())
		return false;
	std::stringstream buffer;
	buffer << file.rdbuf();
	if (file.bad())
        return false;
	body = buffer.str();
	return true;
}

static void	buildStatusLine(std::string &response, const t_responseInstructions &instructions)
{
	std::stringstream						ss;

	ss << instructions.statusCode;
	response =  "HTTP/1.1 " + ss.str() + " " + getReasonPhrase(instructions.statusCode) + "\r\n";
}

static std::string getResponseContentType(
    const t_responseInstructions &instructions,
    const std::map<std::string, std::string> &parsedHeaders)
{
    std::map<std::string, std::string>::const_iterator it;

    it = parsedHeaders.find("content-type");
    if (it != parsedHeaders.end())
        return it->second;
    if ((instructions.statusCode >= 400 && instructions.statusCode <= 599)
        || instructions.isAutoIndex)
        return "text/html";

    return instructions.contentType;
}

static void appendCgiHeaders(
    std::string &response,
    const std::map<std::string, std::string> &parsedHeaders)
{
    for (std::map<std::string, std::string>::const_iterator it = parsedHeaders.begin();
         it != parsedHeaders.end(); ++it)
    {
        if (it->first == "status" || it->first == "content-type" ||
            it->first == "content-length" || it->first == "connection" ||
            it->first == "location")
            continue;

        response += it->first + ": " + it->second + "\r\n";
    }
}

static void	buildHeaders(std::string &response,
	const t_responseInstructions &instructions, const std::string &body,
	const std::map<std::string, std::string> &parsedHeaders)
{
	response += "Content-Type: " + getResponseContentType(instructions, parsedHeaders) + "\r\n";

	std::stringstream len;
	len << body.size();
	response += "Content-Length: " + len.str() + "\r\n";

	std::map<std::string, std::string>::const_iterator location;
	location = parsedHeaders.find("location");

	if (location != parsedHeaders.end())
		response += "Location: " + location->second + "\r\n";
	else if (instructions.statusCode >= 300 && instructions.statusCode < 400 && instructions.isRedirect)
		response += "Location: " + instructions.redirectLocation + "\r\n";

	if (instructions.keepAlive)
		response += "Connection: keep-alive\r\n";
	else
		response += "Connection: close\r\n";

	appendCgiHeaders(response, parsedHeaders);
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

std::string	buildHttpResponse(t_responseInstructions &result,
								 const std::map<int, std::string> &errorPages)
{
	std::string							response;
	std::string							body;
	std::string							cgiHeaders;
	std::map<std::string, std::string>	parsedHeaders;

	if (result.isCgi)
		cgiResponse(result, cgiHeaders, body, parsedHeaders);
	else if (result.method == "DELETE" && result.statusCode == 204)
	{
		if (!deleteFile(result.resolvedPath))
			result.statusCode = 500;
	}
	else if (result.method == "POST" && result.statusCode == 201)
	{
		if (!writePostBody(result.resolvedPath, result.body))
			result.statusCode = 500;
	}
	else if (result.method == "GET" && result.statusCode == 200)
	{
		if (!buildBody(body, result))
			result.statusCode = 500;
	}
	if (result.statusCode >= 400 && result.statusCode <= 599 && (!result.isCgi || !result.cgi.success || parsedHeaders.empty()))
    	body = buildConfiguredErrorBody(result.statusCode, errorPages);
	buildStatusLine(response, result);
	buildHeaders(response, result, body, parsedHeaders);
	response += "\r\n";
	response += body;
	return response;
}
