/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 00:56:34 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/24 17:41:58 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "httpRequestParser.hpp"
#include "httpRequestParserUtils.hpp"

const size_t MAX_HEADER_SIZE = 16 * 1024;

static bool	isHeaderSizeValid(const std::string &buffer)
{
	size_t headerStart = buffer.find("\r\n");
    if (headerStart == std::string::npos)
        return true;
    headerStart += 2;
    size_t headerEnd = buffer.find("\r\n\r\n", headerStart);

    size_t headerSize;
    if (headerEnd == std::string::npos)
        headerSize = buffer.size() - headerStart;
    else
        headerSize = headerEnd + 4 - headerStart;

    return headerSize <= MAX_HEADER_SIZE;
}

static ParseResult parseStartLineStage(const std::string &buffer, HttpRequest &request)
{
    if (!parseStartLine(buffer, request))
        return request.statusCode == 0 ? INCOMPLETE : ERROR;

    request.stage = HEADERS;
    return COMPLETE;
}

static ParseResult parseHeadersStage(const std::string &buffer, HttpRequest &request, size_t maxBodySize)
{
    if (!isHeaderSizeValid(buffer))
    {
        request.statusCode = 431;
        return ERROR;
    }
    size_t headerEnd = buffer.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
	{
		request.statusCode = 0;
        return INCOMPLETE;
	}
    if (!parseHeaders(buffer, request) || !checkRequiredHeaders(request))
    {
        request.statusCode = 400;
        return ERROR;
    }
    request.isChunked = hasTransferEncoding(request);
    if (request.isChunked && !isChunkedTransferEncoding(request))
    {
        request.statusCode = 501;
        return ERROR;
    }
    if (!request.isChunked)
    {
        if (!determineBodyLength(request))
        {
            request.statusCode = 400;
            return ERROR;
        }
        if (request.expectedBodyLength > maxBodySize)
        {
            request.statusCode = 413;
            return ERROR;
        }
    }
    request.bodyStart = headerEnd + 4;
    request.chunkPos = request.bodyStart;
    request.stage = BODY;
    return COMPLETE;
}

ParseResult parseRequest(const std::string &buffer, HttpRequest &request, size_t clientMaxBodySize)
{
	ParseResult result;

    if (request.stage == START_LINE)
    {
        result = parseStartLineStage(buffer, request);
        if (result != COMPLETE)
            return result;
    }
	if (request.stage == HEADERS)
    {
        result = parseHeadersStage(buffer, request, clientMaxBodySize);
        if (result != COMPLETE)
            return result;
    }
	if (request.stage == FINISHED)
        return COMPLETE;

    if (request.isChunked)
        result = parseChunkedBody(buffer, request, clientMaxBodySize);
    else
        result = parseBody(buffer, request);
    if (result == COMPLETE)
        request.stage = FINISHED;
    return result;
}
