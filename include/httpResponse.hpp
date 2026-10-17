/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 10:40:21 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/17 15:42:26 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTPPRESPONSE_HPP
# define HTPPRESPONSE_HPP
# include <string>
# include "serverUtils.hpp"

std::string	buildHttpResponse(const t_responseInstructions &instructions);



#endif