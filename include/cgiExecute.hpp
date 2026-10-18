/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiExecute.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 15:47:37 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/18 11:04:57 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_EXECUTE_HPP
# define CGI_EXECUTE_HPP

# include <vector>
# include <string>

# include "httpRequestParserUtils.hpp"
# include "serverUtils.hpp"
# include "cgiTypes.hpp"

int							executeCgi(const HttpRequest &httpRequest, t_serverState &ctx, const int &fd);
std::vector<std::string>	buildCgiEnvp(HttpRequest &httpRequest);

#endif
