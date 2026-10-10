/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverClean.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/09 11:30:51 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/10 16:57:06 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CLEAN_HPP
# define SERVER_CLEAN_HPP

# include "serverInit.hpp"
# include "serverUtils.hpp"
# include "cgiExecute.hpp"
# include "serverEvent.hpp"

void	closeClientConnection(t_serverState &ctx, int fd, int err);
void	cleanupServ(t_serverState &ctx, t_listeningSockets &listeningSockets);
void	closeListeningSockets(t_listeningSockets &listeningSockets);
void	cleanupCgi(t_cgiProcess &cgiProcess, std::map<int, int> &fdPipeToClient, bool killChild, int *wstatus);

#endif