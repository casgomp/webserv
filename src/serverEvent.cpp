/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverEvent.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 16:50:38 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/16 18:43:47 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <cerrno>
# include <cstring>
# include <fcntl.h>
# include <iostream>
# include <stdexcept>
# include <sys/epoll.h>
# include <sys/socket.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>

# include "serverEvent.hpp"
# include "serverInit.hpp"
# include "serverUtils.hpp"
# include "serverClean.hpp"
# include "httpRequestParser.hpp"
# include "requestRouting.hpp"
# include "requestValidation.hpp"
# include "requestValidationUtils.hpp"
# include "cgiExecute.hpp"
# include "cgiHandle.hpp"
# include "httpResponse.hpp"

void	acceptClient(t_serverState &ctx, int fd, t_listeningSockets &listeningSockets)
{
	struct sockaddr_storage	client_addr;
	socklen_t				addr_size;
	int						clientFd;

	addr_size = sizeof(client_addr);
	clientFd = accept(fd, (struct sockaddr *)&client_addr, &addr_size);
	if (clientFd < 0)
		return ;
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		closeClientConnection(ctx, clientFd, errno);
		return ;
	}
	if (epollSet(ctx.epfd, EPOLL_CTL_ADD, clientFd, EPOLLIN) < 0)
	{
		closeClientConnection(ctx, clientFd, errno);
		return ;
	}
	ctx.clients[clientFd].pairAddressPort = listeningSockets[fd];
	ctx.clients[clientFd].bytesSent = 0;
	ctx.clients[clientFd].keepAlive = true;
	ctx.clients[clientFd].lastActivity = time(NULL);
	ctx.clients[clientFd].waitingForClient = true;
}

void	handleClientSend(t_serverState &ctx, int fd)
{
	int byteCount = 0;
	const std::string &response = ctx.clients[fd].response;
	size_t bytesSent = ctx.clients[fd].bytesSent;
	byteCount = send(fd, response.c_str() + bytesSent, response.size() - bytesSent, 0);
	if (byteCount < 0)
	{
		closeClientConnection(ctx, fd, errno);
		return ;
	}
	ctx.clients[fd].bytesSent += byteCount;
	ctx.clients[fd].lastActivity = time(NULL);
	if (ctx.clients[fd].bytesSent == response.size())
	{
		if (!ctx.clients[fd].keepAlive)
		{
			closeClientConnection(ctx, fd, 0);
			return ;
		}
		ctx.clients[fd].bytesSent = 0;
		ctx.clients[fd].waitingForClient = true;
		ctx.clients[fd].lastActivity = time(NULL);
		if (epollSet(ctx.epfd, EPOLL_CTL_MOD, fd, EPOLLIN) < 0)
		{
			closeClientConnection(ctx, fd, errno);
			return ;
		}
	}
}

void checkClientTimeouts(t_serverState &ctx)
{
    const time_t now = time(NULL);
    const time_t idleTimeout = 60;

    std::map<int, t_client>::iterator it = ctx.clients.begin();

    while (it != ctx.clients.end())
    {
        int fd = it->first;
        t_client &client = it->second;
        ++it;

        if (client.waitingForClient && now - client.lastActivity >= idleTimeout)
            closeClientConnection(ctx, fd, -1);
    }
}

void	handleClientReceive(t_serverState &ctx, int fd, size_t ceilingClientMaxBodySize, t_listenServers &listenServers)
{
	int				byteCount;
	char			buf[BUFFER_SIZE];

	byteCount = recv(fd, buf, sizeof(buf), 0);
	if (byteCount == 0)
	{
		closeClientConnection(ctx, fd, EPOLLIN);
		return ;
	}
	if (byteCount < 0)
	{
		closeClientConnection(ctx, fd, errno);
		return ;
	}
	ctx.clients[fd].lastActivity = time(NULL);
	ctx.clients[fd].request.append(buf, byteCount);
	HttpRequest	httpRequest;
	int requestStatus = parseRequest(ctx.clients[fd].request, httpRequest, ceilingClientMaxBodySize);
	if (requestStatus == INCOMPLETE)
	{
		std::cout << "*********requestStatus: INCOMPLETE" << std::endl;
		return ;
	}
	else if (requestStatus == ERROR)
	{
		std::cout << "*********requestStatus: ERROR" << std::endl;
		ctx.clients[fd].waitingForClient = false;
		// Build and send the appropriate HTTP error response later.
		return;
	}
	else if (requestStatus == COMPLETE)
	{
		std::cout << "*********requestStatus: COMPLETE" << std::endl;
		ctx.clients[fd].request.erase(0, httpRequest.consumedBytes);
		ctx.clients[fd].waitingForClient = false;
		if (httpRequest.headers.find("connection") == httpRequest.headers.end()
			|| httpRequest.headers["connection"] != "close")
		{
			ctx.clients[fd].keepAlive = true;
		}
		else
			ctx.clients[fd].keepAlive = false;
		try
		{
			requestRouting(fd, ctx.clients, listenServers, httpRequest);
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			ctx.clients[fd].response = buildHttpResponse(buildErrorInstructions(500, ctx.clients[fd].serverConf), "requestbody?", "method?");
			ctx.clients[fd].bytesSent = 0;
			ctx.clients[fd].keepAlive = false;
		}
		t_responseInstructions responseInstructions = requestValidation(httpRequest, ctx.clients[fd].serverConf);
		if (responseInstructions.isCgi && cgiStart(ctx, fd, responseInstructions, httpRequest) == 0)
			return ;
	}
	if (epollSet(ctx.epfd, EPOLL_CTL_MOD, fd, EPOLLOUT) < 0)
	{
		closeClientConnection(ctx, fd, errno);
		return ;
	}
}

void	serverEvent(t_listenServers &listenServers, t_listeningSockets &listeningSockets)
{
	t_serverState			ctx;
	int						fd;
	struct epoll_event		evs[MAX_EVENTS];
	int						nreadyfds;
	int						clientFd;
	size_t					ceilingClientMaxBodySize = 0;

	ctx.epfd = epoll_create(1);
	if (ctx.epfd < 0)
	{
		closeListeningSockets(listeningSockets);
		throw std::runtime_error(strerror(errno));
	}
	for (t_listeningSockets::iterator it = listeningSockets.begin(); it != listeningSockets.end(); it ++)
	{
		if (epollSet(ctx.epfd, EPOLL_CTL_ADD, it->first, EPOLLIN) < 0)
		{
			int err = errno;
			cleanupServ(ctx, listeningSockets);
			throw std::runtime_error(strerror(err));
		}
	}
	ceilingClientMaxBodySize = computeCeilingBody(listenServers);
	while (1)
	{
		nreadyfds = epoll_wait(ctx.epfd, evs, MAX_EVENTS, 1000);
		if (nreadyfds < 0)
		{
			cleanupServ(ctx, listeningSockets);
			throw std::runtime_error(strerror(errno));
		}
		for (int i = 0; i < nreadyfds; i++)
		{
			fd = evs[i].data.fd;

			if (listeningSockets.find(fd) != listeningSockets.end())
			{
				if (!(evs[i].events & EPOLLIN))
					continue ;
				acceptClient(ctx, fd, listeningSockets);
			}
			else if (ctx.fdPipeToClient.find(fd) != ctx.fdPipeToClient.end())
			{
				clientFd = ctx.fdPipeToClient[fd];
				if (fd == ctx.clients[clientFd].cgiProcess.stdinFd)
					handleCgiStdin(ctx, fd, clientFd, evs[i].events);
				else if (fd == ctx.clients[clientFd].cgiProcess.stdoutFd)
					handleCgiStdout(ctx, fd, clientFd, evs[i].events);
			}
			else if (ctx.clients.find(fd) != ctx.clients.end())
			{
				/******************************CLIENT: HANDLING REQUEST/SENDING RESPONSE******************************/
				if (evs[i].events & EPOLLERR)
				{
					closeClientConnection(ctx, fd, EPOLLERR);
					continue ;
				}
				else if (evs[i].events & EPOLLHUP)
				{
					closeClientConnection(ctx, fd, EPOLLHUP);
					continue ;
				}
				else if (evs[i].events & EPOLLIN)
					handleClientReceive(ctx, fd, ceilingClientMaxBodySize, listenServers);
				else if (evs[i].events & EPOLLOUT)
					handleClientSend(ctx, fd);
				//when deciding whether to terminate a connection, check also:
				//timeout?
				//http request header connection: keep-alive or close?
				// if (httpRequest.header["connection"] == "close")
				// {
				// 	// close connection
				// }
			}
		}
		checkClientTimeouts(ctx);
	}
}

