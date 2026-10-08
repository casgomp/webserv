/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverEvent.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 16:50:38 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/08 17:45:30 by pecastro         ###   ########.fr       */
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

# include "httpRequestParser.hpp"
# include "serverEvent.hpp"
# include "serverInit.hpp"
# include "serverUtils.hpp"
# include "requestRouting.hpp"
# include "requestValidation.hpp"
# include "requestValidationUtils.hpp"
# include "cgiExecute.hpp"

void	serverEvent(t_listenServers &listenServers, t_listeningSockets &listeningSockets)
{
	int						fd;
	//epoll()
	int						epfd = -1;
	struct epoll_event		ev;
	struct epoll_event		evs[MAX_EVENTS];
	int						nreadyfds;
	//accept();
	struct sockaddr_storage	client_addr;
	socklen_t				addr_size;
	int						fdClient;
	t_client 				new_client;
	std::map<int, t_client>	clients;
	//recv(),send()
	int						byteCount;
	char					buf[BUFFER_SIZE];
	size_t					ceilingClientMaxBodySize = 0;
	// int						bytes_read;
	std::string 			response = "hello from server!";
	int						bytesSent;
	HttpRequest 			httpRequest;
	//cgi
	std::map<int, int>		fdPipeToClient;
	t_cgiProcess			cgiProcess;
	int 					clientFd;
	int						wstatus = 0;
	t_cgiOutput				cgiOutput;
	int						cgiBytesSent;

	(void)listenServers;

	epfd = epoll_create(1);
	if (epfd < 0)
	{
		closeListeningSockets(listeningSockets);
		throw std::runtime_error(strerror(errno));
	}
	for (t_listeningSockets::iterator it = listeningSockets.begin(); it != listeningSockets.end(); it ++)
	{
		ev.events = EPOLLIN;
		ev.data.fd = it->first;
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, it->first, &ev) < 0)
		{
			cleanupServ(listeningSockets, epfd, clients);
			throw std::runtime_error(strerror(errno));
		}
		std::cout << "listening sockets = " << it->second.first << ":" << it->second.second << std::endl;
	}
	for(t_listenServers::iterator it = listenServers.begin(); it != listenServers.end(); it ++)//make helper function
	{
		for (size_t i = 0; i < it->second.size(); i ++)
		{
			if (it->second[i]->ceilingClientMaxBodySize > ceilingClientMaxBodySize)
				ceilingClientMaxBodySize = it->second[i]->ceilingClientMaxBodySize;
		}
	}
	while (1)
	{
		nreadyfds = epoll_wait(epfd, evs, MAX_EVENTS, -1);
		if (nreadyfds < 0)
		{
			cleanupServ(listeningSockets, epfd, clients);
			throw std::runtime_error(strerror(errno));
		}
		for (int i = 0; i < nreadyfds; i++)
		{
			fd = evs[i].data.fd;

			if (listeningSockets.find(fd) != listeningSockets.end())
			{
				/***********************************SERVER: ACCEPT A CONNECTING CLIENT**************************************/
				if (!(evs[i].events & EPOLLIN))
					continue ;
				addr_size = sizeof(client_addr);
				fdClient = accept(fd, (struct sockaddr *)&client_addr, &addr_size);
				if (fdClient < 0)
				{
					closeClientConnection(fdClient, clients, errno);
					continue ;
				}
				if (fcntl(fdClient, F_SETFL, O_NONBLOCK) < 0)
				{
					closeClientConnection(fdClient, clients, errno);
					continue ;
				}
				ev.events = EPOLLIN;
				ev.data.fd = fdClient;
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, fdClient, &ev) < 0)
				{
					closeClientConnection(fdClient, clients, errno);
					continue ;
				}
				clients[fdClient].pairAddressPort = listeningSockets[fd];
				clients[fdClient].bytesSent = 0;

				// clients[fdClient].request.clear();//are these necessary? this is always a new client and therefore a new buffer isn't it?
				// clients[fdClient].response.clear();
			}
			else if (fdPipeToClient.find(fd) != fdPipeToClient.end())
			{
				cgiOutput = t_cgiOutput();
				clientFd = fdPipeToClient[fd];
				if (fd == clients[clientFd].cgiProcess.stdinFd)
				{
					if (evs[i].events & (EPOLLERR | EPOLLHUP))
					{
						close (fd);
						fdPipeToClient.erase(fd);
						clients[clientFd].cgiProcess.stdinFd = -1;
						continue ;
					}
					const std::string &body = clients[clientFd].cgiProcess.body;
					cgiBytesSent = clients[clientFd].cgiProcess.bytesSent;
					byteCount = 0;
					byteCount =	write(fd, body.c_str() + cgiBytesSent, body.size() - cgiBytesSent);
					if (byteCount < 0)
						continue ;
					clients[clientFd].cgiProcess.bytesSent += byteCount;
					if (clients[clientFd].cgiProcess.bytesSent == body.size())
					{
						close (fd);
						fdPipeToClient.erase(fd);
						clients[clientFd].cgiProcess.stdinFd = -1;
						continue ;
					}
				}
				else if (fd == clients[clientFd].cgiProcess.stdoutFd)
				{
					if (evs[i].events & EPOLLERR)
					{
						cleanupCgi(clients[clientFd].cgiProcess, fdPipeToClient, true, NULL);
						closeClientConnection(clientFd, clients, EPOLLERR);
						;//send cgiOutput to Erjon
						continue ;
					}
					byteCount = read(fd, buf, sizeof(buf));
					if (byteCount == 0)
					{
						cleanupCgi(clients[clientFd].cgiProcess, fdPipeToClient, false, &wstatus);
						if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) == 0)
						{
							cgiOutput.buffer = clients[clientFd].cgiProcess.output;
							cgiOutput.success = true;
						}
						;//send cgiOutput to Erjon
						closeClientConnection(clientFd, clients, EPOLLERR);
						continue ;
					}
					else if (byteCount < 0)
					{
						cleanupCgi(clients[clientFd].cgiProcess, fdPipeToClient, true, NULL);
						closeClientConnection(clientFd, clients, EPOLLERR);
						;//send cgiOutput to Erjon
						continue ;
					}
					else if (byteCount > 0)
					{
						clients[clientFd].cgiProcess.output.append(buf, byteCount);
						continue ;
					}
				}
			}
			else
			{
				/******************************CLIENT: HANDLING REQUEST/SENDING RESPONSE******************************/
				if (evs[i].events & EPOLLERR)
				{
					// std::cout << "EPOLLERR: " << std::endl;
					closeClientConnection(fd, clients, EPOLLERR);
					continue ;
				}
				else if (evs[i].events & EPOLLHUP)
				{
					// std::cout << "EPOLLHUP: " << std::endl;
					closeClientConnection(fd, clients, EPOLLHUP);
					continue ;
				}
				else if (evs[i].events & EPOLLIN)
				{
					/********CLIENT: RECEIVE**********/
					 std::cout << "Server ready to receive" << std::endl;
					byteCount = recv(fd, buf, sizeof(buf), 0);
					if (byteCount == 0)
					{
						closeClientConnection(fd, clients, EPOLLIN);
						continue ;
					}
					if (byteCount < 0)
					{
						closeClientConnection(fd, clients, errno);
						continue ;
					}
					clients[fd].request.append(buf, byteCount);
					memset(buf, 0, BUFFER_SIZE);

					/*#############***PARSE AND ROUTING***##############*/
					HttpRequest	httpRequest;
					// std::cout << "!!!!!!!!!!!!!!!!!!clients[fd].request: " << clients[fd].request << std::endl;
					int requestStatus = parseRequest(clients[fd].request, httpRequest, ceilingClientMaxBodySize);
					//int requestStatus = COMPLETE;///////
					std::cout << "*********requestStatus: " << requestStatus << std::endl;
					if (requestStatus == ERROR)
					{
						//prepare response struct with error info.
					}
					if (requestStatus == COMPLETE)
					{
						try
						{
							requestRouting(fd, clients, listenServers, httpRequest);
						}
						catch (const std::exception &e)
						{
							std::cerr << e.what() << std::endl;
							//clients[fd].response = buildErrorResponse(500, "Internal Server Error");//buildErrorResponse is part of response not yet implemented
							clients[fd].bytesSent = 0;
							clients[fd].keepAlive = false;
						}

						t_responseInstructions responseInstructions = requestValidation(httpRequest, clients[fd].serverConf);//create the responseInstructions struct

						// std::cout << "responseInstructions" << std::endl
						// 			<< "statusCode: " << responseInstructions.statusCode << std::endl
						// 			<< "isRedirect: " << responseInstructions.isRedirect << std::endl
						// 			<< "redirectLocation: " << responseInstructions.redirectLocation << std::endl
						// 			<< "isCgi: " << responseInstructions.isCgi << std::endl
						// 			<< "isAutoIndex: " << responseInstructions.isAutoIndex << std::endl
						// 			<< "resolvedPath: " << responseInstructions.resolvedPath << std::endl
						// 			<< "contentType: " << responseInstructions.contentType << std::endl;

						if (responseInstructions.isCgi)
						{
							cgiOutput = t_cgiOutput();
							if (executeCgi(httpRequest, responseInstructions, cgiProcess) != 0)
							{
								;//send cgiOutput to Erjon
								continue ;
							}
							if (fcntl(cgiProcess.stdoutFd, F_SETFL, O_NONBLOCK) < 0)
							{
								int err = errno;
								cleanupCgi(cgiProcess, fdPipeToClient, true, NULL);
								;//send cgiOutput to Erjon
								continue ;
							}
							fdPipeToClient[cgiProcess.stdoutFd] = fd;
							ev.events = EPOLLIN;
							ev.data.fd = cgiProcess.stdoutFd;
							if (epoll_ctl(epfd, EPOLL_CTL_ADD, cgiProcess.stdoutFd, &ev) < 0)
							{
								int err = errno;
								cleanupCgi(cgiProcess, fdPipeToClient, true, NULL);
								;//send cgiOutput to Erjon
								continue ;
							}
							if (cgiProcess.stdinFd != -1)
							{
								if (fcntl(cgiProcess.stdinFd, F_SETFL, O_NONBLOCK) < 0)
								{
									int err = errno;
									cleanupCgi(cgiProcess, fdPipeToClient, true, NULL);
									;//send cgiOutput to Erjon
									continue ;
								}
								fdPipeToClient[cgiProcess.stdinFd] = fd;
								ev.events = EPOLLOUT;
								ev.data.fd = cgiProcess.stdinFd;
								if (epoll_ctl(epfd, EPOLL_CTL_ADD, cgiProcess.stdinFd, &ev) < 0)
								{
									int err = errno;
									cleanupCgi(cgiProcess, fdPipeToClient, true, NULL);
									;//send cgiOutput to Erjon
									continue ;
								}
							}
							clients[fd].cgiProcess = cgiProcess;
							clients[fd].cgiProcess.body = httpRequest.body;
						}

						//what about keep-alive or close at this point?

						//create reponse for client[fd].response = responseCreate(responseInstructions);

						// std::cout << "we received from client: " << clients[fd].request << std::endl;
						
					}
					//else if PARSE_INCOMPLETE, don't do anything.
						//continue;????
					
					ev.events = EPOLLOUT;
					ev.data.fd = fd;
					if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) < 0)
					{
						closeClientConnection(fd, clients, errno);
						continue ;
					}
				}

				else if (evs[i].events & EPOLLOUT)
				{
					/********CLIENT: RESPOND**********/

					//////////////////////////////////////////////////////////test
					char buf2[1024];
					memset(buf2, 0, sizeof(buf2));
					std::cout << "read is happening... " << std::endl;
					read(0, buf2, sizeof(buf2));
					clients[fd].response.append(buf2, strlen(buf2));
					std::cout << "read happened... " << std::endl;
					//////////////////////////////////////////////////////////

					byteCount = 0;
					response = clients[fd].response;
					bytesSent = clients[fd].bytesSent;
					std::cout << "Server ready to send" << std::endl;
					byteCount = send(fd, response.c_str() + bytesSent, response.size() - bytesSent, 0);
					std::cout << "send happend, byte count: " << byteCount << std::endl;
					if (byteCount < 0)
					{
						std::cout << "byte count < 0 " << std::endl;
						closeClientConnection(fd, clients, errno);
						continue ;
					}
					clients[fd].bytesSent += byteCount;
					if (clients[fd].bytesSent == response.size())
					{
						std::cout << "response completed..." << std::endl;
						clients[fd].bytesSent = 0;
						ev.events = EPOLLIN;
						ev.data.fd = fd;
						if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) < 0)
						{
							std::cout << "it will finish on modify..." << std::endl;
							closeClientConnection(fd, clients, errno);
							continue ;
						}
					}
				}
				//when deciding whether to terminate a connection, check also:
				//timeout?
				//http request header connection: keep-alive or close?
				// if (httpRequest.header["connection"] == "close")
				// {
				// 	// close connection
				// }
			}
		}
	}
}

