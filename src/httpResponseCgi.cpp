/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponseCgi.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebara <ebara@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 14:43:09 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/26 16:11:16 by ebara            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpResponse.hpp"
#include "../include/httpRequestParserUtils.hpp"

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

void	cgiResponse(s_responseInstructions &result,
		std::string &cgiHeaders, std::string &body,
		std::map<std::string, std::string> &parsedHeaders)
{
	if (!result.cgi.success)
			result.statusCode = 500;
	else
	{
		if (result.statusCode == 0)
			result.statusCode = 200;
		if (!splitCgiOutput(result.body, cgiHeaders, body))
			result.statusCode = 500;
		else if (!parseCgiHeaders(cgiHeaders, parsedHeaders))
			result.statusCode = 500;
		else
		{
			if (parsedHeaders.find("location") != parsedHeaders.end() && parsedHeaders.find("status") == parsedHeaders.end())
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
