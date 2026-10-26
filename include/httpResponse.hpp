/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebara <ebara@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 10:40:21 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/26 16:36:31 by ebara            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTPPRESPONSE_HPP
# define HTPPRESPONSE_HPP

# include <string>
# include "serverUtils.hpp"
# include <sstream>
# include <map>
# include <fstream>

std::string	getReasonPhrase(int statusCode);

std::string	buildConfiguredErrorBody(int statusCode, const std::map<int,
		std::string> &errorPages);

void		cgiResponse(s_responseInstructions &result,
		std::string &cgiHeaders, std::string &body,
		std::map<std::string, std::string> &parsedHeaders);

std::string	buildHttpResponse(t_responseInstructions &result,
		const std::map<int, std::string> &errorPages);



#endif