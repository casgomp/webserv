/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 00:56:34 by erjonbara         #+#    #+#             */
/*   Updated: 2026/09/14 13:10:03 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTPARSER_HPP
#define HTTPREQUESTPARSER_HPP

#include <string>
#include <map>


struct StartLine
{
    std::string	method;
    std::string	target;
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

ParseResult parseRequest(
    const std::string &buffer,
    HttpRequest &request
);

#endif