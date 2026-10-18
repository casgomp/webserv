/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 10:41:02 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/18 20:59:09 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpResponse.hpp"
#include "../include/httpRequestParserUtils.hpp"
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

static std::string getReasonPhrase(int statusCode)
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

static void	buildStatusLine(std::string &response, const t_responseInstructions &instructions)
{
	std::stringstream						ss;

	ss << instructions.statusCode;
	response =  "HTTP/1.1 " + ss.str() + " " + getReasonPhrase(instructions.statusCode) + "\r\n";
}

static void	 buildHeaders(std::string &response,
	const t_responseInstructions &instructions, const std::string &body,
	const std::map<std::string, std::string> &parsedHeaders)
{
	std::map<std::string, std::string>::const_iterator it;
	it = parsedHeaders.find("content-type");

	if (it != parsedHeaders.end())
		response += "Content-Type: " + it->second + "\r\n";
	else if ((instructions.statusCode >= 400 && instructions.statusCode <= 599) ||
			instructions.isAutoIndex)
		response += "Content-Type: text/html\r\n";
	else
		response += "Content-Type: " + instructions.contentType + "\r\n";
	std::stringstream len;
	len << body.size();
	response += "Content-Length: " + len.str() + "\r\n";
	std::map<std::string, std::string>::const_iterator location;
	location = parsedHeaders.find("location");
	if (location != parsedHeaders.end())
		response += "Location: " + location->second + "\r\n";
	else if (instructions.statusCode >= 300 && instructions.statusCode < 400
			&& instructions.isRedirect)
	{
		response += "Location: " + instructions.redirectLocation + "\r\n";
	}
	if (!instructions.keepAlive)
		response += "Connection: keep-alive\r\n";
	else
		response += "Connection: close\r\n";
	for (std::map<std::string, std::string>::const_iterator it = parsedHeaders.begin();
    	it != parsedHeaders.end(); ++it)
	{
		if (it->first == "status" ||
			it->first == "content-type" ||
			it->first == "content-length" ||
			it->first == "connection" ||
			it->first == "location")
			continue;

		response += it->first + ": " + it->second + "\r\n";
	}
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

static std::string	buildConfiguredErrorBody(int statusCode, const std::map<int, std::string> &errorPages)
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

static bool	splitCgiOutput(const std::string &output, std::string& headers, std::string& body)
{
	size_t	pos = output.find("\r\n\r\n");
	size_t	separatorLen = 4;

	if (pos == std::string::npos)
	{
		pos = output.find("\n\n");
		separatorLen = 2;
	}
	if (pos == std::string::npos)
		return false;
	headers = output.substr(0, pos);
	body = output.substr(pos + separatorLen);
	return true;
}

static bool parseCgiHeaders(const std::string &cgiHeaders,
                            std::map<std::string, std::string> &parsedHeaders)
{
    std::istringstream stream(cgiHeaders);
    std::string line;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        std::string key;
        std::string value;

        if (!parseHeaderLine(line, key, value))
            return false;

        parsedHeaders[key] = value;
    }
    return true;
}

std::string	buildHttpResponse(const t_responseInstructions &instructions,
								 const std::map<int, std::string> &errorPages)
{
	std::string							response;
	std::string							body;
	std::string							cgiHeaders;
	t_responseInstructions				result;
	std::map<std::string, std::string>	parsedHeaders;

	result = instructions;
	if (result.isCgi)
	{
		if (!result.cgi.success)
			result.statusCode = 500;
		else
		{
			if (result.statusCode == 0)
				result.statusCode = 200;
			// decide cgiheaders max size
			if (!splitCgiOutput(result.body, cgiHeaders, body))
				result.statusCode = 500;
			else if (!parseCgiHeaders(cgiHeaders, parsedHeaders))
				result.statusCode = 500;
			else
			{
				if (parsedHeaders.find("location") != parsedHeaders.end()
					&& parsedHeaders.find("status") == parsedHeaders.end())
					result.statusCode = 302;
				std::map<std::string, std::string>::const_iterator it;
				it = parsedHeaders.find("status");

				if (it != parsedHeaders.end())
				{
					std::istringstream ss(it->second);
					int status;

					if (!(ss >> status) || status < 100 || status > 599)
						result.statusCode = 500;
					else
						result.statusCode = status;
				}
			}
		}
	}
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

	if (result.statusCode >= 400 && result.statusCode <= 599
    	&& (!result.isCgi || !result.cgi.success || parsedHeaders.empty()))
	{
    	body = buildConfiguredErrorBody(result.statusCode, errorPages);
	}

	buildStatusLine(response, result);

	buildHeaders(response, result, body, parsedHeaders);

	response += "\r\n";
	response += body;

	return response;
}
