/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 00:56:34 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/06 23:47:13 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_PARSER_HPP
# define HTTP_REQUEST_PARSER_HPP

# include <string>
# include <map>

# include "httpRequestParserUtils.hpp"

ParseResult parseRequest(
    const std::string &buffer, HttpRequest &request, size_t clientMaxBodySize
);

#endif