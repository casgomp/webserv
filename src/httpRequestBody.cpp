/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestBody.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 12:00:31 by erjonbara         #+#    #+#             */
/*   Updated: 2026/09/15 06:41:31 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "httpRequestParserInternal.hpp"
#include <limits>

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
	size_t finalStart = chunkSizeEnd + 2;
	if (buffer.size() - finalStart < 2)
	{
		request.statusCode = 0;
		return INCOMPLETE;
	}
	if (buffer[finalStart] != '\r' || buffer[finalStart + 1] != '\n')
	{
		request.statusCode = 400;
		return ERROR;
	}
	request.consumedBytes = finalStart + 2;
	return COMPLETE;
}

ParseResult	parseChunkedBody(const std::string &buffer, HttpRequest &request)
{
	size_t bodyStart = buffer.find("\r\n\r\n");
    if (bodyStart == std::string::npos)
	{
		request.statusCode = 0;
        return INCOMPLETE;
	}
    bodyStart += 4;
    size_t pos = bodyStart;
    request.body.clear();
    while (true)
    {
        size_t chunkSizeEnd = buffer.find("\r\n", pos);
        if (chunkSizeEnd == std::string::npos)
        {
			request.statusCode = 0;
			return INCOMPLETE;
		}
        std::string chunkSize = buffer.substr(pos, chunkSizeEnd - pos);
        size_t chunkLength;
        if (!hexStringToSizeT(chunkSize, chunkLength))
		{
			request.statusCode = 400;
            return ERROR;
		}
        if (chunkLength == 0)
			return parseFinalChunk(buffer, chunkSizeEnd, request);
        size_t chunkDataStart = chunkSizeEnd + 2;
        if (chunkLength > buffer.size() - chunkDataStart)
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
        request.body.append(buffer, chunkDataStart, chunkLength);
        pos = chunkDataEnd + 2;
    }
}

ParseResult parseBody(const std::string &buffer, HttpRequest &request)
{
    size_t bodyStart = buffer.find("\r\n\r\n");
    if (bodyStart == std::string::npos)
	{
		request.statusCode = 0;
        return INCOMPLETE;
	}
    bodyStart += 4;
    size_t available = buffer.size() - bodyStart;
    if (available < request.expectedBodyLength)
    {
		request.statusCode = 0;
        return INCOMPLETE;
	}
    request.body = buffer.substr(bodyStart, request.expectedBodyLength);
	request.consumedBytes = bodyStart + request.expectedBodyLength;
    return COMPLETE;
}
