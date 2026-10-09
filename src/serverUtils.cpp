/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 14:58:17 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/09 15:37:24 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <sys/epoll.h>
# include <unistd.h>
# include <errno.h>
# include <fcntl.h>

# include "serverUtils.hpp"
# include "cgiExecute.hpp"
# include "serverInit.hpp"
# include "serverClean.hpp"

int	epollSet(int epfd, int operation, int fd, int events)
{
	struct epoll_event	ev;

	ev.events = events;
	ev.data.fd = fd;
	return (epoll_ctl(epfd, operation, fd, &ev));
}

void	closeCgiStdin(t_cgiProcess &cgiProcess, std::map<int, int> &fdPipeToClient)
{
	close (cgiProcess.stdinFd);
	fdPipeToClient.erase(cgiProcess.stdinFd);
	cgiProcess.stdinFd = -1;
}

size_t	computeCeilingBody(const t_listenServers &listenServers)
{
	size_t	ceilingClientMaxBodySize;

	for(t_listenServers::const_iterator it = listenServers.begin(); it != listenServers.end(); it ++)
	{
		for (size_t i = 0; i < it->second.size(); i ++)
		{
			if (it->second[i]->ceilingClientMaxBodySize > ceilingClientMaxBodySize)
				ceilingClientMaxBodySize = it->second[i]->ceilingClientMaxBodySize;
		}
	}
}

void	finishCgiRequest(int epfd, int clientFd, std::map<int, t_client> &clients, t_cgiOutput &cgiOutput)
{
	;//send cgiOutput to Erjon
	if (epollSet(epfd, EPOLL_CTL_MOD, clientFd, EPOLLOUT) < 0)
		closeClientConnection(clientFd, clients, errno);
}

int	registerCgiPipes(int epfd, t_cgiProcess &cgiProcess, int clientFd, std::map<int, int> &fdPipeToClient)
{
	if (fcntl(cgiProcess.stdoutFd, F_SETFL, O_NONBLOCK) < 0)
		return (-1);
	fdPipeToClient[cgiProcess.stdoutFd] = clientFd;
	if (epollSet(epfd, EPOLL_CTL_ADD, cgiProcess.stdoutFd, EPOLLIN) < 0)
		return (-1);
	if (cgiProcess.stdinFd != -1)
	{
		if (fcntl(cgiProcess.stdinFd, F_SETFL, O_NONBLOCK) < 0)
			return (-1);
		fdPipeToClient[cgiProcess.stdinFd] = clientFd;
		if (epollSet(epfd, EPOLL_CTL_ADD, cgiProcess.stdinFd, EPOLLOUT) < 0)
			return (-1);
	}
	return (0);
}