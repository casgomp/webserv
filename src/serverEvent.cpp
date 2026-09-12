/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverEvent.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 16:50:38 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/12 18:51:59 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

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
	int						byte_count;
	char					buf[BUFFER_SIZE];
	// int						bytes_read;
	std::string 			response = "hello from server!";
	int						bytes_sent;

	(void)listenServers;

	epfd = epoll_create(1);
	if (epfd < 0)
	{
		closeListeningSockets(listeningSockets);
		throw std::runtime_error(strerror(errno));
	}
	//ADDING LISTENING SOCKETS WITH EPOLL_CTL SHOULD BE IN LOOP FOR EACH LISTENING SOCKET
	for (t_listeningSockets::iterator it = listeningSockets.begin(); it != listeningSockets.end(); it ++)
	{
		ev.events = EPOLLIN;
		ev.data.fd = it->first;
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, it->first, &ev) < 0)
		{
			cleanupServ(listeningSockets, epfd, clients);
			throw std::runtime_error(strerror(errno));
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
				clients[fdClient].bytes_sent = 0;
				// clients[fdClient].request.clear();//are these necessary? this is always a new client and therefore a new buffer isn't it?
				// clients[fdClient].response.clear();
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
					// std::cout << "Server ready to receive" << std::endl;
					byte_count = recv(fd, buf, sizeof(buf), 1000);
					if (byte_count == 0)
					{
						closeClientConnection(fd, clients, EPOLLIN);
						continue ;
					}
					if (byte_count < 0)
					{
						closeClientConnection(fd, clients, errno);
						continue ;
					}
					clients[fd].request.append(buf, byte_count);
					memset(buf, 0, BUFFER_SIZE);
					
					/*#############***SETUP REQUEST ROUTING***##############*/
						//protocol version = HTTP 1.1
						//path = /
						//method = GET,POST,DELETE
						//host = www.mywebsite.com
						//user-agent = ? curl?
						//accept = ?
						//connection = keep-alive
						//content-type = ?
						//content lentght = ?
						//body?
					HttpRequest httpRequest;
					int requestStatus = parseRequest(clients[fd].request, httpRequest);
					if (requestStatus == PARSE_BAD_REQUEST)
					{
						//prepare response struct with error info.
					}
					if (requestStatus == PARSE_COMPLETE)
					{
						std::pair<std::string, std::string> requestPair = listeningSockets[fd];
						std::vector<t_serverConf *> confServers = listenServers[requestPair];
						std::vector<t_serverConf *> requestPairServers;
						for (i = 0; i < confServers.size(); i ++)
						{
							std::vector<std::pair<std::string, std::string> > serverPairs = (*confServers[i]).listen;
							if (std::find(serverPairs.begin(), serverPairs.end(), requestPair) != serverPairs.end())
							{
								requestPairServers.push_back(confServers[i]);
							}
						}
						clients[fd].serverConf = requestPairServers[0];
						if (requestPairServers.size() > 1)
						{
							for (i = 0; i < requestPairServers.size(); i ++)
							{
								std::vector<std::string> serverNames = (*requestPairServers[i]).serverNames;
								if (std::find(serverNames.begin(), serverNames.end(), httpRequest.headers["host"]) != serverNames.end())
								{
									clients[fd].serverConf = requestPairServers[i];
									break ;
								}
							}
						}
						//VALIDATE (in the following order):
						//1. route-path matching for /fruits, at parsing, even if url contains fruitsaaaa, it's correct. So has
						//to be something like fruitsa, so not matching the full word.
							//404 (Not found)
						//2. allowed methods 
							//400 (Bad request): parsing finds invalid char such as lowercase
							//405 (Method not allowed): no invalid chars, but method does not exist (can also be handled in parsing)
							//403 (Forbidden): when no parsing errors and methods exits, but is not allowed.
						//3. return(redirection) status is specified in the return directive:
							//306 (Temporary Redirect)
							//307 (Permanent Redirect)
						//4. check if path has no trailing '/'
							//if contains try_files then is accepted and goes into possible paths or errors in try_files arguments
							//301 (Moved permanently).
						//5. POST - There's no standard Nginx behavior so we'll implement ours in the following order:
							//413 (Content too large) i.e. compare body size against client_max_body_size
							//415 (Unsuported media type) i.e. compare file.type in request path, against types in our container with
							//supported mime types. Don't compare against content-type in the request header.
							//Check if upload (or whatever name) has permissions and create a file inside and copy body contents:
							//201 (Created)
						//6. DELETE
							//204 (No content) 

						//EXTRAS
							//429 Too many requests.

						// std::cout << "we received from client: " << clients[fd].request << std::endl;

						ev.events = EPOLLOUT;
						ev.data.fd = fd;
						if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) < 0)
						{
							closeClientConnection(fd, clients, errno);
							continue ;
						}
					}
					//else if PARSE_INCOMPLETE, don't do anything.
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


					byte_count = 0;
					response = clients[fd].response;
					bytes_sent = clients[fd].bytes_sent;
					std::cout << "Server ready to send" << std::endl;
					byte_count = send(fd, response.c_str() + bytes_sent, response.size() - bytes_sent, 0);
					std::cout << "send happend, byte count: " << byte_count << std::endl;
					if (byte_count < 0)
					{
						std::cout << "byte count < 0 " << std::endl;
						closeClientConnection(fd, clients, errno);
						continue ;
					}
					clients[fd].bytes_sent += byte_count;
					if (clients[fd].bytes_sent == response.size())
					{
						std::cout << "response completed..." << std::endl;
						clients[fd].bytes_sent = 0;
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
			}
		}
	}
}

