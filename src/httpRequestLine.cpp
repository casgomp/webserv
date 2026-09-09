/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestLine.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 11:04:55 by erjonbara         #+#    #+#             */
/*   Updated: 2026/09/09 12:41:07 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "httpRequestParserInternal.hpp"
#include <cctype>

static int countSpaces(const std::string &str)
{
    int count = 0;

    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
    {
        if (*it == '\t')
            return 0;
        if (*it == ' ')
            count++;
    }
    return count;
}

static bool isValidStartLineFormat(const std::string &line)
{
    return countSpaces(line) == 2;
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

static bool isValidParsedStartLineValues(const StartLine &line)
{
    if (line.method.empty() || line.target.empty() || line.version.empty())
	{
        return false;
	}
	if (!isValidToken(line.method))
		return false;
    if (line.method != "GET" && line.method != "POST" && line.method != "DELETE")
	{
        return false;
	}
	if (!isValidOriginForm(line.target))
		return false;
    if (line.version != "HTTP/1.1")
		return false;
    return true;
}

bool parseStartLine(const std::string &buffer, HttpRequest &request)
{
    size_t end = buffer.find("\r\n");
    if (end == std::string::npos)
        return false;
    std::string line = buffer.substr(0, end);
	if (line.size() > 8000 || line.empty())
		return false;
    if (!isValidStartLineFormat(line))
        return false;
    size_t firstSpace = line.find(' ');
    size_t secondSpace = line.find(' ', firstSpace + 1);
    if (firstSpace == std::string::npos || secondSpace == std::string::npos)
        return false;
    request.requestLine.method = line.substr(0, firstSpace);
    request.requestLine.target =
					line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
    request.requestLine.version = line.substr(secondSpace + 1);
    return isValidParsedStartLineValues(request.requestLine);
}
