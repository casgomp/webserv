/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestHeaders.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 11:47:22 by erjonbara         #+#    #+#             */
/*   Updated: 2026/09/14 22:47:26 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "httpRequestParserInternal.hpp"
#include <limits>
#include <cctype>

static void toLowercase(std::string &str)
{
    for (std::string::iterator it = str.begin(); it != str.end(); ++it)
    {
        *it = std::tolower(static_cast<unsigned char>(*it));
    }
}

static void trimValue(std::string &value)
{
    size_t first = value.find_first_not_of(" \t");
    if (first == std::string::npos)
    {
        value.clear();
        return;
    }
    size_t last = value.find_last_not_of(" \t");
    value = value.substr(first, last - first + 1);
}


static bool isValidValue(const std::string &value)
{
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
    {
        unsigned char c = static_cast<unsigned char>(*it);
        if (c == '\t')
            continue;
        if (c >= 32 && c <= 126)
            continue;
        return false;
    }
    return true;
}

static bool isValidContentLength(const std::string &value)
{
    if (value.empty())
        return false;
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
    {
        if (*it < '0' || *it > '9')
            return false;
    }
    return true;
}

static bool parseHeaderLine(const std::string &line, std::string &key, std::string &value)
{
	size_t colon = line.find(':');
	if (colon == std::string::npos)
		return false;
	key = line.substr(0, colon);
	value = line.substr(colon + 1);
	if (!isValidToken(key))
		return false;
	toLowercase(key);
	trimValue(value);
	if (!isValidValue(value))
		return false;
	return true;
}

static bool handleDuplicateHeader(HttpRequest &request, const std::string &key, const std::string &value)
{
	std::map<std::string, std::string>::iterator it;
	it = request.headers.find(key);
	if (it == request.headers.end())
	{
		request.headers[key] = value;
		return true;
	}
	if (key == "host")
		return false;
	if (key == "transfer-encoding")
		return false;
	if (key == "content-length")
	{
		if (it->second != value)
			return false;
		return true;
	}
	it->second += ", " + value;
	return true;
}

bool parseHeaders(const std::string &buffer, HttpRequest &request)
{
    size_t start = buffer.find("\r\n");
    size_t end = buffer.find("\r\n\r\n");
    if (start == std::string::npos || end == std::string::npos)
        return false;
    start += 2;
    std::string headersBlock = buffer.substr(start, end - start);
    size_t pos = 0;
    while (pos < headersBlock.size())
    {
        size_t lineEnd = headersBlock.find("\r\n", pos);
        std::string line;
        if (lineEnd == std::string::npos)
        {
            line = headersBlock.substr(pos);
            pos = headersBlock.size();
        }
        else
        {
            line = headersBlock.substr(
                pos,
                lineEnd - pos);
            pos = lineEnd + 2;
        }
		std::string	key;
		std::string	value;
        if (!parseHeaderLine(line, key, value))
			return false;
        if ((key == "content-length" && !isValidContentLength(value)))
            return false;
		if (key == "content-length" && request.headers.find("transfer-encoding") != request.headers.end())
			return false;
		if (key == "transfer-encoding" && request.headers.find("content-length") != request.headers.end())
			return false;
		if (!handleDuplicateHeader(request, key, value))
			return false;
    }
    return true;
}

bool checkRequiredHeaders(const HttpRequest &request)
{
    std::map<std::string, std::string>::const_iterator it;

    it = request.headers.find("host");

    if (it == request.headers.end())
        return false;

    if (it->second.empty())
        return false;

    return true;
}

bool hasTransferEncoding(const HttpRequest &request)
{
	if (request.headers.find("transfer-encoding") == request.headers.end())
		return false;
	return true;
}

bool isChunkedTransferEncoding(const HttpRequest &request)
{
	if (!hasTransferEncoding(request))
		return false;
	std::string value;
	value = request.headers.find("transfer-encoding")->second;
	toLowercase(value);
	if (value != "chunked")
		return false;
	return true;
}

static bool stringToSizeT(
    const std::string &value,
    size_t &out)
{
    if (value.empty())
        return false;
    size_t result = 0;
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
	{
		if (*it < '0' || *it > '9')
			return false;
		size_t digit = static_cast<size_t>(*it - '0');
		if (result >
			(std::numeric_limits<size_t>::max() - digit) / 10)
			return false;
		result = result * 10 + digit;
	}
    out = result;
    return true;
}

bool determineBodyLength(HttpRequest &request)
{
    request.expectedBodyLength = 0;
    std::map<std::string, std::string>::iterator it;

    it = request.headers.find("content-length");
    if (it == request.headers.end())
        return true;
    return stringToSizeT(it->second, request.expectedBodyLength);
}
