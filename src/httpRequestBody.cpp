/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestBody.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 12:00:31 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/24 17:37:45 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <limits>
# include <string>

#include "httpRequestParserUtils.hpp"

static int	hexDigitValue(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static bool hexStringToSizeT(const std::string &str, size_t &out)
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

static ParseResult parseFinalChunk(const std::string &buffer, size_t chunkSizeEnd, HttpRequest &request)
{
	size_t pos = chunkSizeEnd + 2;

	while (true)
	{
		size_t lineEnd = buffer.find("\r\n", pos);
		if (lineEnd == std::string::npos)
		{
			request.statusCode = 0;
			return INCOMPLETE;
		}
		if (lineEnd == pos)
		{
			request.consumedBytes = lineEnd + 2;
			return COMPLETE;
		}
		std::string line = buffer.substr(pos, lineEnd - pos);
		std::string key;
		std::string value;
		if (!parseHeaderLine(line, key, value))
		{
			request.statusCode = 400;
			return ERROR;
		}
		pos = lineEnd + 2;
	}
}

static ParseResult parseChunkSize(const std::string &buffer, size_t pos,
    size_t &chunkSizeEnd, size_t &chunkLength, HttpRequest &request)
{
    chunkSizeEnd = buffer.find("\r\n", pos);
    if (chunkSizeEnd == std::string::npos)
    {
        request.statusCode = 0;
        return INCOMPLETE;
    }
    std::string chunkSize = buffer.substr(pos, chunkSizeEnd - pos);
    size_t semicolon = chunkSize.find(';');
    if (semicolon != std::string::npos)
        chunkSize = chunkSize.substr(0, semicolon);
    if (!hexStringToSizeT(chunkSize, chunkLength))
    {
        request.statusCode = 400;
        return ERROR;
    }
    return COMPLETE;
}

static ParseResult appendChunkData(const std::string &buffer, HttpRequest &request,
	size_t chunkDataStart, size_t chunkLength, size_t maxBodySize)
{
    if (chunkDataStart > buffer.size() || chunkLength > buffer.size() - chunkDataStart)
    {
        request.statusCode = 0;
        return INCOMPLETE;
    }
    size_t chunkDataEnd = chunkDataStart + chunkLength;
    if (buffer.size() - chunkDataEnd < 2)
    {
        request.statusCode = 0;
        return INCOMPLETE;
    }
    if (buffer[chunkDataEnd] != '\r' || buffer[chunkDataEnd + 1] != '\n')
    {
        request.statusCode = 400;
        return ERROR;
    }
    if (request.body.size() > maxBodySize || chunkLength > maxBodySize - request.body.size())
    {
        request.statusCode = 413;
        return ERROR;
    }
    request.body.append(buffer, chunkDataStart, chunkLength);
    request.chunkPos = chunkDataEnd + 2;
    return COMPLETE;
}

ParseResult	parseChunkedBody(const std::string &buffer, HttpRequest &request, size_t clientMaxBodySize)
{
	while (true)
    {
        size_t		chunkSizeEnd;
        size_t		chunkLength;
        ParseResult	result;

		result = parseChunkSize(buffer, request.chunkPos, chunkSizeEnd, chunkLength, request);
        if (result != COMPLETE)
            return result;
        if (chunkLength == 0)
            return parseFinalChunk(buffer, chunkSizeEnd, request);
        result = appendChunkData(buffer, request, chunkSizeEnd + 2, chunkLength, clientMaxBodySize);
        if (result != COMPLETE)
            return result;
    }
}

ParseResult parseBody(const std::string &buffer, HttpRequest &request)
{
    if (buffer.size() - request.bodyStart < request.expectedBodyLength)
    {
        request.statusCode = 0;
        return INCOMPLETE;
    }

    request.body = buffer.substr(request.bodyStart, request.expectedBodyLength);
    request.consumedBytes = request.bodyStart + request.expectedBodyLength;
    return COMPLETE;
}
