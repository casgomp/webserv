/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiHandle.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/10 13:10:47 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/18 11:12:16 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLE_HPP
# define CGI_HANDLE_HPP

# include <sys/types.h>

# include "serverEvent.hpp"
# include "requestValidationUtils.hpp"
# include "httpRequestParserUtils.hpp"
# include "cgiTypes.hpp"

int		cgiStart(t_serverState &ctx, int fd, const HttpRequest &httpRequest);
void	handleCgiStdin(t_serverState &ctx, int fd, int clientFd, const uint32_t &events);
void	handleCgiStdout(t_serverState &ctx, int fd, int clientFd, const uint32_t &events);
void	closeCgiStdin(t_cgiProcess &cgiProcess, std::map<int, int> &fdPipeToClient);
void	finishCgiRequest(t_serverState &ctx, int clientFd);
int		registerCgiPipes(t_serverState &ctx, t_cgiProcess &cgiProcess, int clientFd);

#endif

