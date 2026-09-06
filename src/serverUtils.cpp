/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 14:58:17 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/04 18:17:29 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

void	closeConnection(int fd, std::map<int, t_client> &clients, int flag_err)
{
	if (fd >= 0)
	{
		clients.erase(fd);
		close(fd);
	}

	if (flag_err == EPOLLERR)
		std::cerr << "Connection: Error condition happened on the associated file descriptor." << std::endl;
	else if (flag_err == EPOLLHUP)
		std::cerr << "Connection: Abrupt close happened on the associated file descriptor" << std::endl;
	else if (flag_err == EPOLLIN)
		std::cerr << "Connection: Graceful close happened on the associated file descriptor" << std::endl;
	else
		std::cerr << "Error: " << strerror(flag_err) << std::endl; 
	//WHAT ABOUT TIMEOUT? WHAT KIND OF DISCONNECTION IS THAT?
}

void	cleanupServ(int servsock, int epfd, std::map<int, t_client> &clients, int flag_err)
{
	std::cerr << "Error: " << flag_err <<std::endl;
	if (servsock >= 0)
		close (servsock); //will have to handle multiple servsockets in the future, so here should also loop
	if (epfd >= 0)
		close (epfd);
	std::map<int, t_client>::iterator it = clients.begin();
	while (it != clients.end())
	{
		close(it->first);
		it ++;
	}
}
