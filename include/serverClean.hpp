/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverClean.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/09 11:30:51 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/09 12:08:07 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CLEAN_HPP
# define SERVER_CLEAN_HPP

# include "serverInit.hpp"
# include "serverUtils.hpp"
# include "cgiExecute.hpp"

void	closeClientConnection(int fd, std::map<int, t_client> &clients, int flag_err);
void	cleanupServ(t_listeningSockets &listeningSockets, int epfd, std::map<int, t_client> &clients);
void	closeListeningSockets(t_listeningSockets &listeningSockets);
void	cleanupCgi(t_cgiProcess &cgiProcess, std::map<int, int> &fdPipeToClient, bool killChild, int *wstatus);

#endif
