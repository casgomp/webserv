
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define PORT "3490"
#include <netinet/in.h>
#include <arpa/inet.h>

int	server()
{
	int							status;
	struct addrinfo				hints;
	struct addrinfo 			*servinfo;
	struct addrinfo				*p;
	int							servsock;
	int							backlog = 5; //max length of pending connections queue.
	int							clientsock;
	struct sockaddr_storage		client_addr;
	socklen_t					addr_size;
	//send(), recv() stuff below:
	int							bytes_sent;
	char 						msg[] = 
									"HTTP/1.1 200 OK\r\n"
									"Content-Type: text/plain\r\n"
									"Content-Length: 12\r\n"
									"\r\n"
									"Hello World!";
	int							bytes_received;
	char						buf[1024];
	// poll()
	// struct pollfd			*pfds;
	std::vector<struct pollfd>	pfds;

	/********************************************************* */
	/*CREATE ARRAY TO STORE ALL SOCKET FDS (LISTENING + CLIENT)*/
	// int	fd_size = 5;
	// int	fd_count = 0;
	// pfds = new pollfd[fd_size];//should be a vector in c++
	//if new failed return 1.
	
	/****************************************************** */
	/*CREATE THE LISTENING SOCKET, BIND IT AND SET TO LISTEN*/
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
		servsock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (servsock < 0)
			continue ;
		std::cout << "servinfo->ai_protocol = " << p->ai_protocol << std::endl;
		std::cout << "servinfo->ai_addrlen = " << p->ai_addrlen << std::endl;
		int yes = 1;
		setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		//fcntl(servsock, F_SETFL, O_NONBLOCK);
		if (bind(servsock, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(servsock);
			continue ;
		}
		break ;
	}
	if (p == NULL)
		return (1);
	//probably should loop through servinfo until finds one that works?
	freeaddrinfo(servinfo);
	if (listen(servsock, backlog) == -1)
	{
		close(servsock);
		return (1);
	}
	struct pollfd	tmp_pfd;
	tmp_pfd.fd = servsock;
	tmp_pfd.events = POLLIN;
	pfds.push_back(tmp_pfd);
	// fd_count = 1;

	/******************************************************* */
	/*START THE POLL LOOP*/
	while (1)
	{
		// int poll_count = poll(pfds, fd_count, -1);
		int poll_count = poll(&pfds[0], pfds.size(), -1);
		if (poll_count == -1)
		{
			//close all sockets and clean any memory allocated.
			return (1);
		}

		unsigned long i = -1;
		// while (++i < fd_count)
		while (++i < pfds.size())
		{
			if (pfds[i].revents & (POLLIN | POLLHUP))
			{
				if (pfds[i].fd == servsock)
				{
					/******************************************************* */
					/*ACCEPT A CONNECTING CLIENT AND STORE FD IN ARRAY OF FDS*/
					addr_size = sizeof(client_addr);
					clientsock = accept(servsock, (struct sockaddr *)&client_addr, &addr_size);
					if (clientsock == -1)
					{
						close(servsock);//close all the existing client sockets as well?
						return (1);
					}
					//here must make socket non-blocking with fcntl
					struct pollfd	tmp_pfd;
					memset(&tmp_pfd, 0, sizeof(tmp_pfd));
					tmp_pfd.fd = clientsock;
					tmp_pfd.events = POLLIN;
					pfds.push_back(tmp_pfd);
				}
				else
				{
					/******************************************************* */
					/*HANDLE CLIENT DATA*/
					// len = strlen(buf);
					clientsock = pfds[i].fd;
					memset(buf, 0, sizeof(buf));
					bytes_received = recv(clientsock, buf, sizeof(buf), 0);
					std::cout << "n bytes recvd from client " << pfds[i].fd << "= " << bytes_received << std::endl;
					if (bytes_received <= 0)
					{
						if (bytes_received == 0)
							std::cout << "pollserver: socket " << clientsock << "hung up." << std::endl;
						else
							std::cout << "Error: " << strerror(errno) << std::endl;
						close(pfds[i].fd);
						pfds[i] = pfds.back();
						pfds.pop_back();
						i --;
					}
					else
					{
						std::cout << "pollserver: this is what we got:\n" << buf << std::endl;
						buf[bytes_received] = '\0';
						if (strncmp(buf, "GET ", 4) == 0 || strncmp(buf, "POST ", 5) == 0 ||
									strncmp(buf, "PUT ", 4) == 0 || strncmp(buf, "HEAD ", 5) == 0 ||
									strncmp(buf, "DELETE ", 7) == 0)
						{
							// looks like a real HTTP request — respond with HTTP message
							send(pfds[i].fd, msg, strlen(msg), 0);
						}
						else
						{
							for (unsigned long i = 0; i < pfds.size(); i ++)
							{
								if (pfds[i].fd != servsock && pfds[i].fd != clientsock)
								{	
									bytes_sent = send(pfds[i].fd, buf, bytes_received, 0);
									if (bytes_sent == -1)
									{
										//close stuff;
										return (1);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if (close(servsock) == -1)
		return (1);
	if (close(clientsock) == -1)
		return (1);
	return (0);
}


// 1 getaddrinfo();
// 2 socket();
// 3 bind();
// 4 listen();
// 5 accept();
// 6 poll()?
	//send() returns bytes sent. not always all sent in one go.
	//receive() return=0 means client ended connection.
	//close()

// - If you don’t want a socket to be blocking, you have to make a call to fcntl()
// By setting it to non-blocking, you can effectively “poll” it for information
// (subject file) Checking the value of errno to adjust the server behaviour 
// is strictly forbidden after performing a read or write operation:
// - If you try to read from a non-blocking socket and there’s no data there, 
// it’s not allowed to block—it will return -1 and errno will be set to 
// EAGAIN or EWOULDBLOCK 
// Generally speaking, however, this type of polling is a bad idea. If you put 
// your program in a busy-wait looking for data on the socket, you’ll suck up 
// CPU time like it was going out of style. A more elegant solution for 
// checking to see if there’s data waiting to be read comes in the 
// following section on poll()
// The general gameplan is to keep an array of struct pollfd s with information 
// about which socket descriptors we want to monitor, and what kind of events we 
// want to monitor for. The OS will block on the poll() call 


