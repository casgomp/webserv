/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiExecute.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 15:47:37 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/04 15:43:16 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_EXECUTION_HPP
# define CGI_EXECUTION_HPP

# include <vector>
# include <string>

# include "httpRequestParserUtils.hpp"

std::vector<std::string>	buildCgiEnvp(HttpRequest &httpRequest);

#endif
