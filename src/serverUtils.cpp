/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 14:58:17 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/07 13:51:36 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

void	closeClientConnection(int fd, std::map<int, t_client> &clients, int err)
{
	if (fd >= 0)
	{
		clients.erase(fd);
		close(fd);
	}
	if (err == EPOLLERR)
		std::cerr << "Connection: Error condition happened on the associated file descriptor." << std::endl;
	else if (err == EPOLLHUP)
		std::cerr << "Connection: Abrupt close happened on the associated file descriptor" << std::endl;
	else if (err == EPOLLIN)
		std::cerr << "Connection: Graceful close happened on the associated file descriptor" << std::endl;
	else
		std::cerr << "Error: " << strerror(err) << std::endl; 
	//WHAT ABOUT TIMEOUT? WHAT KIND OF DISCONNECTION IS THAT?
}

void	closeListeningSockets(t_listeningSockets &listeningSockets)
{
	for (t_listeningSockets::iterator it = listeningSockets.begin(); it != listeningSockets.end(); it ++)
	{
		// std::cout << "cleanupServ cleaned fd = " << it->first << std::endl;
		if (it->first >= 0)
			close (it->first);
	}
}

void	cleanupServ(t_listeningSockets &listeningSockets, int epfd, std::map<int, t_client> &clients)
{
	closeListeningSockets(listeningSockets);
	if (epfd >= 0)
		close (epfd);
	for (std::map<int, t_client>::iterator it = clients.begin(); it != clients.end(); it ++)
		close(it->first);
}