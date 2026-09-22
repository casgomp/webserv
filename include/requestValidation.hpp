/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestValidation.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:15:29 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/22 14:32:11 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_VALIDATION_HPP
# define REQUEST_VALIDATION_HPP

// # include "main.hpp"
# include "httpRequestParser.hpp"
# include "configConf.hpp"
# include "requestValidationUtils.hpp"

t_responseInstructions	requestValidation(HttpRequest &httpRequest, t_serverConf *serverConf);

#endif
