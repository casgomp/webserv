/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverEvent.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:14:42 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/10 17:11:51 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_EVENT_HPP
# define SERVER_EVENT_HPP

# include "serverUtils.hpp"
# include "serverInit.hpp"

#define MAX_EVENTS 64
#define BUFFER_SIZE 1024

//epfd, clients and fdPipeToClient
typedef struct	s_serverState {
	int						epfd;
	std::map<int, t_client>	clients;
	std::map<int, int>		fdPipeToClient;
	s_serverState() : epfd (-1) {}
} t_serverState;

void	serverEvent(t_listenServers &listenServers, t_listeningSockets &listeningSockets);
void	handleClientReceive(t_serverState &ctx, int fd, size_t ceilingClientMaxBodySize, t_listenServers &listenServers);
void	handleClientSend(t_serverState &ctx, int fd);
void	acceptClient(t_serverState &ctx, int fd, t_listeningSockets &listeningSockets);

#endif