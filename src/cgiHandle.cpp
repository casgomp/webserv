/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiHandle.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/10 13:10:29 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/18 15:04:01 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <errno.h>
# include <stdint.h>
# include <sys/epoll.h>
# include <unistd.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <sys/types.h>

# include "cgiHandle.hpp"
# include "serverUtils.hpp"
# include "requestValidationUtils.hpp"
# include "httpRequestParserUtils.hpp"
# include "cgiExecute.hpp"
# include "serverClean.hpp"
# include "serverEvent.hpp"

void	closeCgiStdin(t_cgiProcess &cgiProcess, std::map<int, int> &fdPipeToClient)
{
	if (cgiProcess.stdinFd < 0)
		return ;
	close (cgiProcess.stdinFd);
	fdPipeToClient.erase(cgiProcess.stdinFd);
	cgiProcess.stdinFd = -1;
}

void	finishCgiRequest(t_serverState &ctx, int clientFd)
{
	if (ctx.clients[clientFd].responseInstructions.cgi.success == false)
		resetErrorInstructions(ctx, clientFd, 500);
	resetErrorInstructions(ctx, clientFd, 500);
	if (epollSet(ctx.epfd, EPOLL_CTL_MOD, clientFd, EPOLLOUT) < 0)
		closeClientConnection(ctx, clientFd, errno);
}

int	registerCgiPipes(t_serverState &ctx, t_cgiProcess &cgiProcess, int clientFd)
{
	if (fcntl(cgiProcess.stdoutFd, F_SETFL, O_NONBLOCK) < 0)
		return (-1);
	ctx.fdPipeToClient[cgiProcess.stdoutFd] = clientFd;
	if (epollSet(ctx.epfd, EPOLL_CTL_ADD, cgiProcess.stdoutFd, EPOLLIN) < 0)
		return (-1);
	if (cgiProcess.stdinFd != -1)
	{
		if (fcntl(cgiProcess.stdinFd, F_SETFL, O_NONBLOCK) < 0)
			return (-1);
		ctx.fdPipeToClient[cgiProcess.stdinFd] = clientFd;
		if (epollSet(ctx.epfd, EPOLL_CTL_ADD, cgiProcess.stdinFd, EPOLLOUT) < 0)
			return (-1);
	}
	return (0);
}

void	handleCgiStdin(t_serverState &ctx, int fd, int clientFd, const uint32_t &events)
{
	int		byteCount;
	size_t	cgiBytesSent;

	if (events & (EPOLLERR | EPOLLHUP))
	{
		closeCgiStdin(ctx.clients[clientFd].cgiProcess, ctx.fdPipeToClient);
		return ;
	}
	const std::string &body = ctx.clients[clientFd].cgiProcess.body;
	cgiBytesSent = ctx.clients[clientFd].cgiProcess.bytesSent;
	byteCount =	write(fd, body.c_str() + cgiBytesSent, body.size() - cgiBytesSent);
	if (byteCount < 0)
	{
		cleanupCgi(ctx.clients[clientFd].cgiProcess, ctx.fdPipeToClient, true, NULL);
		finishCgiRequest(ctx, clientFd);
		return ;
	}
	ctx.clients[clientFd].cgiProcess.bytesSent += byteCount;
	if (ctx.clients[clientFd].cgiProcess.bytesSent == body.size())
	{
		closeCgiStdin(ctx.clients[clientFd].cgiProcess, ctx.fdPipeToClient);
		return ;
	}
}

void	handleCgiStdout(t_serverState &ctx, int fd, int clientFd, const uint32_t &events)
{
	char		buf[BUFFER_SIZE];
	int			byteCount;
	int			wstatus = -1;

	if (events & EPOLLERR)
	{
		cleanupCgi(ctx.clients[clientFd].cgiProcess, ctx.fdPipeToClient, true, NULL);
		finishCgiRequest(ctx, clientFd);
		return ;
	}
	byteCount = read(fd, buf, sizeof(buf));
	if (byteCount == 0)
	{
		ctx.clients[clientFd].responseInstructions.body = ctx.clients[clientFd].cgiProcess.output;
		cleanupCgi(ctx.clients[clientFd].cgiProcess, ctx.fdPipeToClient, false, &wstatus);
		if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) == 0 && !ctx.clients[clientFd].responseInstructions.body.empty())
			ctx.clients[clientFd].responseInstructions.cgi.success = true;
		if (ctx.clients[clientFd].responseInstructions.cgi.success == false)
			resetErrorInstructions(ctx, clientFd, 500);
		finishCgiRequest(ctx, clientFd);
		return ;
	}
	else if (byteCount < 0)
	{
		cleanupCgi(ctx.clients[clientFd].cgiProcess, ctx.fdPipeToClient, true, NULL);
		finishCgiRequest(ctx, clientFd);
		return ;
	}
	else if (byteCount > 0)
	{
		ctx.clients[clientFd].cgiProcess.output.append(buf, byteCount);
		return ;
	}
}

int	cgiStart(t_serverState &ctx, int fd, const HttpRequest &httpRequest)
{
	if (executeCgi(httpRequest, ctx, fd) != 0)
	{
		resetErrorInstructions(ctx, fd, 500);
		return (-1);
	}
	else if (registerCgiPipes(ctx, ctx.clients[fd].cgiProcess, fd) < 0)
	{
		cleanupCgi(ctx.clients[fd].cgiProcess, ctx.fdPipeToClient, true, NULL);
		resetErrorInstructions(ctx, fd, 500);
		return (-1);
	}
	else
	{
		ctx.clients[fd].cgiProcess = ctx.clients[fd].cgiProcess;
		ctx.clients[fd].cgiProcess.body = httpRequest.body;
		if (epollSet(ctx.epfd, EPOLL_CTL_MOD, fd, 0) < 0)
		{
			int err = errno;
			cleanupCgi(ctx.clients[fd].cgiProcess, ctx.fdPipeToClient, true, NULL);
			closeClientConnection(ctx, fd, err);
		}
	}
	return (0);
}
