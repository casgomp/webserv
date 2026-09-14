/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestParserInternal.hpp                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 11:36:52 by erjonbara         #+#    #+#             */
/*   Updated: 2026/09/09 12:05:55 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTPARSERINTERNAL_HPP
# define HTTPREQUESTPARSERINTERNAL_HPP

# include "../include/httpRequestParser.hpp"

bool isValidToken(const std::string &token);

bool parseStartLine(
	const std::string &buffer,
	HttpRequest &request
);

bool parseHeaders(
    const std::string &buffer,
    HttpRequest &request);

bool checkRequiredHeaders(
    const HttpRequest &request);

bool hasTransferEncoding(const HttpRequest &request);

bool isChunkedTransferEncoding(const HttpRequest &request);

bool determineBodyLength(HttpRequest &request);

ParseResult	parseChunkedBody(
	const std::string &buffer,
	HttpRequest &request
);

ParseResult parseBody(
    const std::string &buffer,
    HttpRequest &request);

#endif