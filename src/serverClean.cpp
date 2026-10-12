/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverClean.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/09 11:30:19 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/10 16:59:43 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <cstring>
# include <iostream>
# include <map>
# include <signal.h>
# include <sys/epoll.h>
# include <sys/wait.h>
# include <unistd.h>

# include "serverUtils.hpp"
# include "serverInit.hpp"
# include "cgiExecute.hpp"
# include "serverEvent.hpp"

void	cleanupCgi(t_cgiProcess &cgiProcess, std::map<int, int> &fdPipeToClient, bool killChild, int *wstatus)
{
	if (cgiProcess.pid > 0)
	{
		if (killChild)
			kill(cgiProcess.pid, SIGKILL);
		waitpid(cgiProcess.pid, wstatus, 0);
	}
	if (cgiProcess.stdinFd >= 0)
		close(cgiProcess.stdinFd);
	if (cgiProcess.stdoutFd >= 0)
		close(cgiProcess.stdoutFd);
	fdPipeToClient.erase(cgiProcess.stdinFd);
	fdPipeToClient.erase(cgiProcess.stdoutFd);
	cgiProcess = t_cgiProcess();
}

void	closeClientConnection(t_serverState &ctx, int fd, int err)
{
	if (fd >= 0)
	{
		if (ctx.clients.find(fd) != ctx.clients.end() && ctx.clients[fd].cgiProcess.pid > 0)
			cleanupCgi(ctx.clients[fd].cgiProcess, ctx.fdPipeToClient, true, NULL);
		ctx.clients.erase(fd);
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

void	cleanupServ(t_serverState &ctx, t_listeningSockets &listeningSockets)
{
	closeListeningSockets(listeningSockets);
	if (ctx.epfd >= 0)
		close (ctx.epfd);
	for (std::map<int, t_client>::iterator it = ctx.clients.begin(); it != ctx.clients.end(); it ++)
	{
		if  (it->second.cgiProcess.pid > 0)
			cleanupCgi(it->second.cgiProcess, ctx.fdPipeToClient, true, NULL);
		close(it->first);
	}
}

