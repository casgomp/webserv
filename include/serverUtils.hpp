/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverUtils.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:15:04 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/09 15:37:26 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_UTILS_HPP
# define SERVER_UTILS_HPP

# include <utility>
# include <string>

# include "configConf.hpp"
# include "configConf.hpp"
# include "cgiExecute.hpp"

typedef struct	s_client {
	std::pair<std::string, std::string>	pairAddressPort;
	t_serverConf						*serverConf;
	t_cgiProcess						cgiProcess;
	std::string							request;
	std::string							response;
	size_t								bytesSent;
	bool								keepAlive;
} t_client;

int		epollSet(int epfd, int operation, int fd, int events);
void	closeCgiStdin(t_cgiProcess &cgiProcess, std::map<int, int> &fdPipeToClient);
size_t	computeCeilingBody(const t_listenServers &listenServers);
void	finishCgiRequest(int epfd, int clientFd, std::map<int, t_client> &clients, t_cgiOutput &cgiOutput);
int		registerCgiPipes(int epfd, t_cgiProcess &cgiProcess, int clientFd, std::map<int, int> &fdPipeToClient);
void	acceptClient(int epfd, int fd, std::map<int, t_client> &clients, t_listeningSockets &listeningSockets);

#endif