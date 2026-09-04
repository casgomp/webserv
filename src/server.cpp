/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 11:39:09 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/04 15:34:44 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netdb.h>
#include <string>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "3490"
#define MAX_EVENTS 64
#define BUFFER_SIZE 1024

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

int	server()//server should receive the data structure containing the info from the config file.
{
	//server socket: getaddrinfo(), socket(), bind(), listen();
	int						status;
	struct addrinfo			hints;
	struct addrinfo			*servinfo;
	struct addrinfo			*p;
	int						servsock = -1;//change name to listening socket maybe should be a map<int, pair<str,str> >? to easily find based on fd.
	int						yes;
	int						backlog = 32;
	struct sockaddr_storage	client_addr;
	socklen_t				addr_size;
	//epoll()
	int						epfd = -1;
	struct epoll_event		ev;
	struct epoll_event		evs[MAX_EVENTS];
	int						nreadyfds;
	//accept();
	int						clientsock;
	t_client 				new_client;
	std::map<int, t_client>	clients;
	//recv(),send()
	int						byte_count;
	char					buf[BUFFER_SIZE];
	// int						bytes_read;
	std::string 			response = "hello from server!";
	int						bytes_sent;


//THIS SHOULD BE ON A LOOP TO CREATE MULTIPLE SERVERS IF NECESSARY, AND ALL THEIR SPECIFIED LISTENING PORTS(i.e. MUTIPLE LISTENING SOCKETS IN SAME SERVER)
//for getaddrinfo, check manually: char *port ... if the address is "" or "*" then port == NULL....else port = addr.c_str();
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
	{
		std::cerr << gai_strerror(status) << std::endl;
		return (1);
	}
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		servsock = socket(p->ai_family, p->ai_socktype | SOCK_NONBLOCK, p->ai_protocol);
		if (servsock < 0)
			continue;
		yes = 1;
		if (setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
		{
			close(servsock);
			std::cerr << "Error: " << strerror(errno) <<std::endl;
			continue ;
		}
		if (bind(servsock, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(servsock);
			std::cerr << "Error: " << strerror(errno) <<std::endl;
			continue ;
		}
		if (listen(servsock, backlog) < 0)
		{
			close(servsock);
			std::cerr << "Error: " << strerror(errno) <<std::endl;
			continue ;
		}
		break ;
	}
	freeaddrinfo(servinfo);
	if (p == NULL)
		return (1);
	/*******************************************************************/
	/*EPOLL*/
	/*******************************************************************/
	epfd = epoll_create(1);
	if (epfd < 0)
	{
		close(servsock);
		std::cerr << "Error: " << strerror(errno) <<std::endl;
		return (1);
	}
	//THIS SHOULD ALSO BE A WHILE LOOP TO ADD WITH EPOLL_CTL ALL THE LISTENING SERVER SOCKETS (I.E. ALL LISTENING PORTS)
	ev.events = EPOLLIN;
	ev.data.fd = servsock;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, servsock, &ev) < 0)
	{
		cleanupServ(servsock, epfd, clients, errno);
		return(1);
	}
	while (1)
	{
		nreadyfds = epoll_wait(epfd, evs, MAX_EVENTS, -1);
		if (nreadyfds < 0)
		{
			cleanupServ(servsock, epfd, clients, errno);
			return (1);
		}
		for (int i = 0; i < nreadyfds; i++)
		{
			/********SERVER: ACCEPT A CONNECTING CLIENT**********/
			if (evs[i].data.fd == servsock)
			{
				if (!(evs[i].events & EPOLLIN))
					continue ;
				addr_size = sizeof(client_addr);
				clientsock = accept(servsock, (struct sockaddr *)&client_addr, &addr_size);
				if (clientsock < 0)
				{
					closeConnection(clientsock, clients, errno);
					continue ;
				}
				if (fcntl(clientsock, F_SETFL, O_NONBLOCK) < 0)
				{
					closeConnection(clientsock, clients, errno);
					continue ;
				}
				ev.events = EPOLLIN;
				ev.data.fd = clientsock;
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientsock, &ev) < 0)
				{
					closeConnection(clientsock, clients, errno);
					continue ;
				}
				clients[clientsock] = new_client;
				clients[clientsock].fd = clientsock;
				// store the client[clientsock].pairAddressPort too ...evs[i].data.fd contains the server socket fd associated to this client

				// clients[clientsock].request.clear();
				// clients[clientsock].response.clear();
				clients[clientsock].bytes_sent = 0;
			}
			else
			{
				/********CLIENT**********/
				if (evs[i].events & EPOLLERR)
				{
					std::cout << "EPOLLERR: " << std::endl;
					closeConnection(evs[i].data.fd, clients, EPOLLERR);
					continue ;
				}
				else if (evs[i].events & EPOLLHUP)
				{
					std::cout << "EPOLLHUP: " << std::endl;
					closeConnection(evs[i].data.fd, clients, EPOLLHUP);
					continue ;
				}
				else if (evs[i].events & EPOLLIN)
				{
					/********CLIENT: RECEIVE**********/
					std::cout << "Server ready to receive" << std::endl;
					byte_count = recv(evs[i].data.fd, buf, sizeof(buf), 0);
					if (byte_count == 0)
					{
						closeConnection(evs[i].data.fd, clients, EPOLLIN);
						continue ;
					}
					if (byte_count < 0)
					{
						closeConnection(evs[i].data.fd, clients, errno);
						continue ;
					}
					clients[evs[i].data.fd].request.append(buf, byte_count);
					memset(buf, 0, BUFFER_SIZE);
					int request_complete = 1;//should be a function call
					//parse request to check for r/n/r/n/
					if (request_complete)
					{
						std::cout << "we received from client: " << clients[evs[i].data.fd].request << std::endl;
						ev.events = EPOLLOUT;
						ev.data.fd = evs[i].data.fd;
						if (epoll_ctl(epfd, EPOLL_CTL_MOD, evs[i].data.fd, &ev) < 0)
						{
							closeConnection(evs[i].data.fd, clients, errno);
							continue ;
						}
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
					clients[evs[i].data.fd].response.append(buf2, strlen(buf2));
					std::cout << "read happened... " << std::endl;
					//////////////////////////////////////////////////////////

					byte_count = 0;
					response = clients[evs[i].data.fd].response;
					bytes_sent = clients[evs[i].data.fd].bytes_sent;
					std::cout << "Server ready to send" << std::endl;
					byte_count = send(evs[i].data.fd, response.c_str() + bytes_sent, response.size() - bytes_sent, 0);
					std::cout << "send happend, byte count: " << byte_count << std::endl;
					if (byte_count < 0)
					{
						std::cout << "byte count < 0 " << std::endl;
						closeConnection(evs[i].data.fd, clients, errno);
						continue ;
					}
					clients[evs[i].data.fd].bytes_sent += byte_count;
					if (clients[evs[i].data.fd].bytes_sent == response.size())
					{
						std::cout << "response completed..." << std::endl;
						clients[evs[i].data.fd].bytes_sent = 0;
						ev.events = EPOLLIN;
						ev.data.fd = evs[i].data.fd;
						if (epoll_ctl(epfd, EPOLL_CTL_MOD, evs[i].data.fd, &ev) < 0)
						{
							std::cout << "it will finish on modify..." << std::endl;
							closeConnection(evs[i].data.fd, clients, errno);
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

// struct epoll_event {
//     uint32_t events;    // Bitmask of events (EPOLLIN, EPOLLOUT...)
//     epoll_data_t data;  // A union for user data (FD, ptr, uint64...)
// };

// typedef union epoll_data {
//     void     *ptr;
//     int       fd;
//     uint32_t  u32;
//     uint64_t  u64;
// } epoll_data_t;

