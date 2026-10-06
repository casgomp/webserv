/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestParserUtils.hpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 11:36:52 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/07 00:13:47 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_PARSER_UTILS_HPP
# define HTTP_REQUEST_PARSER_UTILS_HPP

# include <map>
# include <string>

struct StartLine
{
    std::string	method;
    std::string	target;
	std::string	queryString;
    std::string	version;
};

struct HttpRequest
{
    StartLine							requestLine;
    std::map<std::string, std::string>	headers;
    std::string							body;
    size_t								expectedBodyLength;
    size_t								consumedBytes;
	int									statusCode;
};

enum ParseResult
{
    COMPLETE,
    INCOMPLETE,
    ERROR
};

bool isValidToken(const std::string &token);
bool parseStartLine(const std::string &buffer, HttpRequest &request);
bool parseHeaderLine(const std::string &line, std::string &key, std::string &value);
bool parseHeaders(const std::string &buffer, HttpRequest &request);
bool checkRequiredHeaders(const HttpRequest &request);
bool hasTransferEncoding(const HttpRequest &request);
bool isChunkedTransferEncoding(const HttpRequest &request);
bool determineBodyLength(HttpRequest &request);
ParseResult	parseChunkedBody(const std::string &buffer, HttpRequest &request, size_t clientMaxBodySize);
ParseResult parseBody(const std::string &buffer, HttpRequest &request);

#endif