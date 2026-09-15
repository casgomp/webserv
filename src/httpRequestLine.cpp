/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestLine.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 11:04:55 by erjonbara         #+#    #+#             */
/*   Updated: 2026/09/15 07:13:43 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "httpRequestParserInternal.hpp"
#include <cctype>

static const size_t MAX_REQUEST_LINE = 8000;

static bool isValidStartLineFormat(const std::string &line)
{
    int count = 0;

    for (std::string::const_iterator it = line.begin(); it != line.end(); ++it)
    {
        if (*it == '\t')
            return false;
        if (*it == ' ')
            count++;
    }
    if (count != 2)
		return false;
	return true;
}

bool isValidToken(const std::string &token)
{
    if (token.empty())
        return false;
    std::string allowed = "!#$%&'*+-.^_`|~";
    for (std::string::const_iterator it = token.begin();
         it != token.end(); ++it)
    {
        unsigned char c = static_cast<unsigned char>(*it);
        if (std::isalnum(c))
            continue;
        if (allowed.find(*it) != std::string::npos)
            continue;
        return false;
    }
    return true;
}

static bool isHexDigit(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static bool isValidPercentEncoding(const std::string &str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '%')
		{
			if (i + 2 >= str.size())
				return false;
			if (!isHexDigit(str[i + 1]) || !isHexDigit(str[i + 2]))
				return false;
			i += 2;
		}
	}
	return true;
}

static bool isUnreserved(char c)
{
	unsigned char uc = static_cast<unsigned char>(c);

	if (std::isalnum(uc) || (uc == '-') || uc == '.' || uc == '_' || uc == '~')
		return true;
	return false;
}

static bool isSubDelimiter(char c)
{
	std::string	subDelimiter = "!$&'()*+,;=";
	if (subDelimiter.find(c) != std::string::npos)
		return true;
	return false;
}

static bool isValidPathChar(char c)
{
	if (!isUnreserved(c) && !isSubDelimiter(c) && c != ':' && c != '@' && c != '/')
		return false;
	return true;

}

static bool isValidQuery(char c)
{
	if (!isValidPathChar(c) && c != '?')
		return false;
	return true;
}

static bool isValidOriginForm(const std::string &target)
{
	if (target.empty() || *target.begin() != '/')
		return false;
	if (target.find(' ') != std::string::npos)
		return false;
	if (!isValidPercentEncoding(target))
		return false;
	size_t i = 0;
	for (i = 0; i < target.size(); i++)
	{
		if (target[i] == '%')
			continue;
		if (target[i] == '?')
			break;
		if (!isValidPathChar(target[i]))
			return false;
	}
	for (i += 1; i < target.size(); i++)
	{
		if (target[i] == '%')
			continue;
		if (!isValidQuery(target[i]))
			return false;
	}
	return true;
}

static bool isValidHttpVersion(const std::string &version)
{
	if (version.length() != 8)
		return false;
	if (version.substr(0, 5) != "HTTP/")
		return false;
	char check = version[5];
	if (check < '0'  || check > '9' )
		return false;
	check = version[6];
	if (check != '.')
		return false;
	check = version[7];
	if (check < '0'  || check > '9' )
		return false;
	return true;
}

static bool isValidParsedStartLineValues(int &statusCode, const StartLine &line)
{
    if (line.method.empty() || line.target.empty() || line.version.empty())
	{
		statusCode = 400;
        return false;
	}
	if (!isValidToken(line.method))
	{
		statusCode = 400;
        return false;
	}
    if (line.method != "GET" && line.method != "POST" && line.method != "DELETE")
	{
		statusCode = 501;
        return false;
	}
	if (!isValidOriginForm(line.target))
	{
		statusCode = 400;
		return false;
	}
	if (!isValidHttpVersion(line.version))
	{
		statusCode = 400;
		return false;
	}
    if (line.version != "HTTP/1.1")
	{
		statusCode = 505;
		return false;
	}
    return true;
}

bool parseStartLine(const std::string &buffer, HttpRequest &request)
{
    size_t end = buffer.find("\r\n");
    if (end == std::string::npos)
	{
		if (buffer.find('\n') != std::string::npos)
		{
			request.statusCode = 400;
			return false;
		}
		if (buffer.size() > MAX_REQUEST_LINE)
			request.statusCode = 414;
		else
			request.statusCode = 0;
		return false;
	}
    std::string line = buffer.substr(0, end);
	if (line.empty())
	{
		request.statusCode = 400;
		return false;
	}
	if (line.size() > MAX_REQUEST_LINE)
	{
		request.statusCode = 414;
		return false;
	}
    if (!isValidStartLineFormat(line))
	{
		request.statusCode = 400;
        return false;
	}
    size_t firstSpace = line.find(' ');
    size_t secondSpace = line.find(' ', firstSpace + 1);
    request.requestLine.method = line.substr(0, firstSpace);
    request.requestLine.target =
					line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
    request.requestLine.version = line.substr(secondSpace + 1);
	if (!isValidParsedStartLineValues(request.statusCode, request.requestLine))
        return false;
	return true;
}
