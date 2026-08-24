/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 00:56:34 by erjonbara         #+#    #+#             */
/*   Updated: 2026/08/24 18:55:45 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <map>
#include <string>
#include <cctype>
#include <limits>

struct StartLine
{
    std::string method;
    std::string target;
    std::string version;
};

struct HttpRequest
{
    StartLine requestLine;
    std::map<std::string, std::string> headers;
    std::string body;
    size_t expectedBodyLength;
	size_t	consumedBytes;
};

enum ParseResult
{
    PARSE_COMPLETE,
    PARSE_INCOMPLETE,
    PARSE_BAD_REQUEST
};


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

bool isValidParsedStartLineValues(const StartLine &line)
{
    if (line.method.empty()
        || line.target.empty()
        || line.version.empty())
        return false;

    if (line.method != "GET"
        && line.method != "POST"
        && line.method != "DELETE")
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

void normalizeKey(std::string &key)
{
    for (std::string::iterator it = key.begin();
         it != key.end(); ++it)
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

bool isValidKeySyntax(const std::string &key)
{
    if (key.empty())
        return false;

    std::string allowed = "!#$%&'*+-.^_`|~";

    for (std::string::const_iterator it = key.begin();
         it != key.end(); ++it)
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

        if (!isValidKeySyntax(key))
            return false;

        normalizeKey(key);

        if (request.headers.find(key)
            != request.headers.end())
            return false;

        trimValue(value);

        if (!isValidValue(value))
            return false;

        if (key == "content-length"
            && !isValidContentLength(value))
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
        return PARSE_BAD_REQUEST;

    if (!determineBodyLength(request))
        return PARSE_BAD_REQUEST;

    return parseBody(buffer, request);
}

int main()
{
    std::string buffer =
        "POST /first HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "Hello"
        "GET /second HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    HttpRequest request;

    ParseResult result = parseRequest(buffer, request);

    if (result == PARSE_COMPLETE)
    {
        std::cout << "FIRST REQUEST COMPLETE\n";
        std::cout << "Target: " << request.requestLine.target << "\n";
        std::cout << "Body: " << request.body << "\n";
        std::cout << "Consumed: " << request.consumedBytes << "\n";

        buffer.erase(0, request.consumedBytes);

        std::cout << "\n--- BUFFER LEFT ---\n";
        std::cout << buffer << "\n";
    }
	result = parseRequest(buffer, request);
	if (result == PARSE_COMPLETE)
    {
        std::cout << "SECOND REQUEST COMPLETE\n";
        std::cout << "Target: " << request.requestLine.target << "\n";
        std::cout << "Body: " << request.body << "\n";
        std::cout << "Consumed: " << request.consumedBytes << "\n";

        buffer.erase(0, request.consumedBytes);

        std::cout << "\n--- BUFFER LEFT ---\n";
        std::cout << buffer << "\n";
    }

    return 0;
}

