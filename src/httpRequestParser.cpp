/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 00:56:34 by erjonbara         #+#    #+#             */
/*   Updated: 2026/09/09 12:39:13 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "httpRequestParserInternal.hpp"

ParseResult parseRequest(const std::string &buffer, HttpRequest &request)
{
    request.expectedBodyLength = 0;
    request.headers.clear();
    request.body.clear();
	request.consumedBytes = 0;
    if (buffer.find("\r\n\r\n") == std::string::npos)
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
