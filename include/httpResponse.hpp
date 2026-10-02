/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 10:40:21 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/01 22:40:00 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTPPRESPONSE_HPP
# define HTPPRESPONSE_HPP
# include <string>
# include "webserv.hpp"

std::string	buildHttpResponse(const t_responseInstructions &instructions,
							const std::string &requestBody,
							const std::string &method);



#endif