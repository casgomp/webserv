/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 00:56:34 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/22 14:40:28 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "httpRequestParser.hpp"
#include "httpRequestParserUtils.hpp"
#include <iostream>

const size_t MAX_HEADER_SIZE = 16 * 1024;

void	clearRequest(HttpRequest &request)
{
	request.requestLine.method.clear();
	request.requestLine.target.clear();
	request.requestLine.queryString.clear();
	request.requestLine.version.clear();
    request.headers.clear();
    request.body.clear();
	request.expectedBodyLength = 0;
	request.consumedBytes = 0;
	request.statusCode = 0;
}

static bool	isHeaderSizeValid(const std::string &buffer)
{
	size_t headerStart = buffer.find("\r\n") + 2;
	size_t headerEnd = buffer.find("\r\n\r\n", headerStart);

	size_t headerSize;

	if (headerEnd == std::string::npos)
		headerSize = buffer.size() - headerStart;
	else
		headerSize = headerEnd + 4 - headerStart;

	if (headerSize > MAX_HEADER_SIZE)
		return false;
	return true;
}

ParseResult parseRequest(const std::string &buffer, HttpRequest &request, size_t clientMaxBodySize, 
	size_t &parseStage, size_t &bytesValidated)
{
	if (parseStage == 0)
	{
		if (!parseStartLine(buffer, request))
		{
			if (request.statusCode == 0)
				return INCOMPLETE;
			std::cout << "\nreturn error\n";
			return ERROR;
		}
		parseStage = 1;
	}
	if (parseStage == 1)
	{
		if (parseStage == 1 && !isHeaderSizeValid(buffer))
		{
			request.statusCode = 431;
			return ERROR;
		}
		if (buffer.find("\r\n\r\n") == std::string::npos)
		{
			request.statusCode = 0;
			return INCOMPLETE;
		}
		parseStage = 2;
		if (!parseHeaders(buffer, request))
		{
			request.statusCode = 400;
			return ERROR;
		}
		if (!checkRequiredHeaders(request))
		{
			request.statusCode = 400;
			return ERROR;
		}
		bytesValidated = buffer.find("\r\n\r\n") + 4;
	}
	if (hasTransferEncoding(request))
	{
		if (!isChunkedTransferEncoding(request))
		{
			request.statusCode = 501;
			return ERROR;
		}
		return parseChunkedBody(buffer, request, clientMaxBodySize);
	}
    if (!determineBodyLength(request))
	{
		request.statusCode = 400;
        return ERROR;
	}
	if (request.expectedBodyLength > clientMaxBodySize)
	{
		request.statusCode = 413;
        return ERROR;
	}
    return parseBody(buffer, request);
}
