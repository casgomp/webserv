/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverUtils.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:15:04 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/08 17:49:28 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_UTILS_HPP
# define SERVER_UTILS_HPP

# include "serverInit.hpp"

typedef struct	s_client {
	std::pair<std::string, std::string>	pairAddressPort;
	t_serverConf						*serverConf;
	t_cgiProcess						cgiProcess;
	std::string							request;
	std::string							response;
	size_t								bytesSent;
	bool								keepAlive;
} t_client;

typedef struct	s_cgiOutput {
	bool		success;
	std::string	buffer;
	s_cgiOutput() : success (false) {}
} t_cgiOutput;

void	closeClientConnection(int fd, std::map<int, t_client> &clients, int flag_err);
void	cleanupServ(t_listeningSockets &listeningSockets, int epfd, std::map<int, t_client> &clients);
void	closeListeningSockets(t_listeningSockets &listeningSockets);
void	cleanupCgi(t_cgiProcess &cgiProcess, std::map<int, int> &fdPipeToClient, bool killChild, int *wstatus);

#endif