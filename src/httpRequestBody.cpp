/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestBody.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 12:00:31 by erjonbara         #+#    #+#             */
/*   Updated: 2026/09/09 12:32:27 by erjonbara        ###   ########.fr       */
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

        std::string chunkSize = buffer.substr(pos, chunkSizeEnd - pos);
        size_t chunkLength;
        if (!hexStringToSizeT(chunkSize, chunkLength))
            return PARSE_BAD_REQUEST;
        if (chunkLength == 0)
        {
            size_t finalStart = chunkSizeEnd + 2;
            if (buffer.size() < finalStart + 2)
                return PARSE_INCOMPLETE;
            if (buffer[finalStart] != '\r' || buffer[finalStart + 1] != '\n')
                return PARSE_BAD_REQUEST;
            request.consumedBytes = finalStart + 2;
            return PARSE_COMPLETE;
        }
        size_t chunkDataStart = chunkSizeEnd + 2;
        if (buffer.size() < chunkDataStart + chunkLength)
            return PARSE_INCOMPLETE;
        size_t chunkDataEnd = chunkDataStart + chunkLength;
        if (buffer.size() < chunkDataEnd + 2)
            return PARSE_INCOMPLETE;
        if (buffer[chunkDataEnd] != '\r' || buffer[chunkDataEnd + 1] != '\n')
            return PARSE_BAD_REQUEST;
        request.body.append(buffer, chunkDataStart, chunkLength);
        pos = chunkDataEnd + 2;
    }
}

ParseResult parseBody(const std::string &buffer, HttpRequest &request)
{
    size_t bodyStart = buffer.find("\r\n\r\n");
    if (bodyStart == std::string::npos)
        return PARSE_INCOMPLETE;
    bodyStart += 4;
    size_t available = buffer.size() - bodyStart;
    if (available < request.expectedBodyLength)
        return PARSE_INCOMPLETE;
    request.body = buffer.substr(bodyStart, request.expectedBodyLength);
	request.consumedBytes = bodyStart + request.expectedBodyLength;
    return PARSE_COMPLETE;
}
