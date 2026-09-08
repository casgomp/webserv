/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 00:56:34 by erjonbara         #+#    #+#             */
/*   Updated: 2026/09/08 11:37:53 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpRequestParser.hpp"

#include <cctype>
#include <limits>
#include <iostream>

int countSpaces(const std::string &str)
{
    int count = 0;

    for (std::string::const_iterator it = str.begin();
         it != str.end(); ++it)
    {
        if (*it == '\t')
            return 0;

        if (*it == ' ')
            count++;
    }

    return count;
}

bool isValidStartLineFormat(const std::string &line)
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
        unsigned char c =
            static_cast<unsigned char>(*it);

        if (std::isalnum(c))
            continue;

        if (allowed.find(*it) != std::string::npos)
            continue;

        return false;
    }

    return true;
}

bool isHexDigit(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool isValidPercentEncoding(const std::string &str)
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

bool isUnreserved(char c)
{
	unsigned char uc = static_cast<unsigned char>(c);

	if (std::isalnum(uc) || (uc == '-') || uc == '.' || uc == '_' || uc == '~')
		return true;
	return false;
}

bool isSubDelimiter(char c)
{
	std::string	subDelimiter = "!$&'()*+,;=";
	if (subDelimiter.find(c) != std::string::npos)
		return true;
	return false;
}

bool isValidPathChar(char c)
{
	if (!isUnreserved(c) && !isSubDelimiter(c) && c != ':' && c != '@' && c != '/')
		return false;
	return true;

}

bool isValidQuery(char c)
{
	if (!isValidPathChar(c) && c != '?')
		return false;
	return true;
}

bool isValidOriginForm(const std::string &target)
{
	if (target.empty())
		return false;
	if (*target.begin() != '/')
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

bool isValidParsedStartLineValues(const StartLine &line)
{
    if (line.method.empty()
        || line.target.empty()
        || line.version.empty())
	{
        return false;
	}
	if (!isValidToken(line.method))
		return false;

    if (line.method != "GET"
        && line.method != "POST"
        && line.method != "DELETE")
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

    if (firstSpace == std::string::npos
        || secondSpace == std::string::npos)
        return false;

    request.requestLine.method =
        line.substr(0, firstSpace);

    request.requestLine.target =
        line.substr(firstSpace + 1,
                    secondSpace - firstSpace - 1);

    request.requestLine.version =
        line.substr(secondSpace + 1);

    return isValidParsedStartLineValues(request.requestLine);
}

void toLowercase(std::string &str)
{
    for (std::string::iterator it = str.begin();
         it != str.end(); ++it)
    {
        *it = std::tolower(
            static_cast<unsigned char>(*it));
    }
}

void trimValue(std::string &value)
{
    size_t first = value.find_first_not_of(" \t");

    if (first == std::string::npos)
    {
        value.clear();
        return;
    }

    size_t last = value.find_last_not_of(" \t");

    value = value.substr(
        first,
        last - first + 1
    );
}


bool isValidValue(const std::string &value)
{
    for (std::string::const_iterator it = value.begin();
         it != value.end(); ++it)
    {
        unsigned char c =
            static_cast<unsigned char>(*it);

        if (c == '\t')
            continue;

        if (c >= 32 && c <= 126)
            continue;

        return false;
    }

    return true;
}

bool isValidContentLength(const std::string &value)
{
    if (value.empty())
        return false;

    for (std::string::const_iterator it = value.begin();
         it != value.end(); ++it)
    {
        if (*it < '0' || *it > '9')
            return false;
    }

    return true;
}

bool stringToSizeT(
    const std::string &value,
    size_t &out)
{
    if (value.empty())
        return false;

    size_t result = 0;

    for (std::string::const_iterator it = value.begin();
    	it != value.end(); ++it)
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

bool parseHeaders(
    const std::string &buffer,
    HttpRequest &request)
{
    size_t start = buffer.find("\r\n");
    size_t end = buffer.find("\r\n\r\n");

    if (start == std::string::npos
        || end == std::string::npos)
        return false;

    start += 2;

    std::string headersBlock =
        buffer.substr(start, end - start);

    size_t pos = 0;

    while (pos < headersBlock.size())
    {
        size_t lineEnd =
            headersBlock.find("\r\n", pos);

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

        size_t colon = line.find(':');

        if (colon == std::string::npos)
            return false;

        std::string key =
            line.substr(0, colon);

        std::string value =
            line.substr(colon + 1);

        if (!isValidToken(key))
            return false;

        toLowercase(key);

		if (key != "content-length" && request.headers.find(key)
            != request.headers.end())
		{
			return false;
		}

        trimValue(value);

        if (!isValidValue(value))
            return false;

        if ((key == "content-length"
            && !isValidContentLength(value)))
            return false;


		std::map<std::string, std::string>::iterator it;
		it = request.headers.find(key);
		if (it != request.headers.end())
		{
			if (it->second != value)
				return false;
		}
		if (key == "content-length" && request.headers.find("transfer-encoding") != request.headers.end())
			return false;
		if (key == "transfer-encoding" && request.headers.find("content-length") != request.headers.end())
			return false;
        request.headers[key] = value;
    }

    return true;
}

bool checkRequiredHeaders(
    const HttpRequest &request)
{
    std::map<std::string, std::string>::const_iterator it;

    it = request.headers.find("host");

    if (it == request.headers.end())
        return false;

    if (it->second.empty())
        return false;

    return true;
}

bool determineBodyLength(HttpRequest &request)
{
    request.expectedBodyLength = 0;

    std::map<std::string, std::string>::iterator it;

    it = request.headers.find("content-length");

    if (it == request.headers.end())
        return true;

    return stringToSizeT(
        it->second,
        request.expectedBodyLength
    );
}

ParseResult parseBody(
    const std::string &buffer,
    HttpRequest &request)
{
    size_t bodyStart =
        buffer.find("\r\n\r\n");

    if (bodyStart == std::string::npos)
        return PARSE_INCOMPLETE;

    bodyStart += 4;

    size_t available =
        buffer.size() - bodyStart;

    if (available < request.expectedBodyLength)
        return PARSE_INCOMPLETE;

    request.body = buffer.substr(
        bodyStart,
        request.expectedBodyLength
    );

	request.consumedBytes = bodyStart + request.expectedBodyLength;

    return PARSE_COMPLETE;
}

int	hexDigitValue(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';

	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

bool hexStringToSizeT(const std::string &str, size_t &out)
{
    if (str.empty())
        return false;

    size_t result = 0;

    for (size_t i = 0; i < str.size(); i++)
    {
        int digit = hexDigitValue(str[i]);

        if (digit == -1)
            return false;

        if (result > (std::numeric_limits<size_t>::max() - digit) / 16)
            return false;

        result = result * 16 + digit;
    }

    out = result;
    return true;
}

ParseResult	parseChunkedBody(const std::string &buffer, HttpRequest &request)
{
	 size_t bodyStart = buffer.find("\r\n\r\n");

    if (bodyStart == std::string::npos)
        return PARSE_INCOMPLETE;

    bodyStart += 4;

    size_t pos = bodyStart;
    request.body.clear();

    while (true)
    {
        size_t chunkSizeEnd = buffer.find("\r\n", pos);

        if (chunkSizeEnd == std::string::npos)
            return PARSE_INCOMPLETE;

        std::string chunkSize =
            buffer.substr(pos, chunkSizeEnd - pos);

        size_t chunkLength;

        if (!hexStringToSizeT(chunkSize, chunkLength))
            return PARSE_BAD_REQUEST;

        if (chunkLength == 0)
        {
            size_t finalStart = chunkSizeEnd + 2;

            if (buffer.size() < finalStart + 2)
                return PARSE_INCOMPLETE;

            if (buffer[finalStart] != '\r'
                || buffer[finalStart + 1] != '\n')
            {
                return PARSE_BAD_REQUEST;
            }

            request.consumedBytes = finalStart + 2;

            return PARSE_COMPLETE;
        }

        size_t chunkDataStart = chunkSizeEnd + 2;

        if (buffer.size() < chunkDataStart + chunkLength)
            return PARSE_INCOMPLETE;

        size_t chunkDataEnd =
            chunkDataStart + chunkLength;

        if (buffer.size() < chunkDataEnd + 2)
            return PARSE_INCOMPLETE;

        if (buffer[chunkDataEnd] != '\r'
            || buffer[chunkDataEnd + 1] != '\n')
        {
            return PARSE_BAD_REQUEST;
        }

        request.body.append(
            buffer,
            chunkDataStart,
            chunkLength
        );

        pos = chunkDataEnd + 2;
    }
}

ParseResult parseRequest(
    const std::string &buffer,
    HttpRequest &request)
{
    request.expectedBodyLength = 0;
    request.headers.clear();
    request.body.clear();
	request.consumedBytes = 0;

    if (buffer.find("\r\n\r\n")
        == std::string::npos)
        return PARSE_INCOMPLETE;

    if (!parseStartLine(buffer, request))
        return PARSE_BAD_REQUEST;

    if (!parseHeaders(buffer, request))
        return PARSE_BAD_REQUEST;

    if (!checkRequiredHeaders(request))
	{
        return PARSE_BAD_REQUEST;
	}
	if (hasTransferEncoding(request))
	{
		if (!isChunkedTransferEncoding(request))
		{
			return PARSE_BAD_REQUEST;
		}
		return parseChunkedBody(buffer, request);
	}

    if (!determineBodyLength(request))
        return PARSE_BAD_REQUEST;

    return parseBody(buffer, request);
}
