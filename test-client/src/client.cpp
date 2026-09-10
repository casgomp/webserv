/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 15:08:51 by pecastro          #+#    #+#             */
/*   Updated: 2026/08/10 16:46:50 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/client.hpp"

#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>

//#define PORT "3490"
#define PORT "8080"
#define MAX_EVENTS 64 ///////
#define BUFFER_SIZE 1024 ////////

// static const std::string readRequest()
// {
// 	std::ifstream	fs;
// 	std::string		line;
// 	std::string		request;

// 	fs.open("requests/request.txt", std::ios::in);
// 	if (!fs.is_open())
// 		throw std::invalid_argument("Error: could not open file");
// 	while(getline(fs, line))
// 	{
// 		request.append(line);
// 	}
// 	std::cout << request << std::endl;
// 	return(request);
// }

int	client()
{
	int						status;
	struct addrinfo			hints;
	struct addrinfo			*clientinfo;
	struct addrinfo			*p;
	int						csock = -1;
	//std::string				request = readRequest();
	//std::string				request = "hello world!";
	// std::string				request = "GET / HTTP/1.1\r\n"
	// 									"Host: localhost\r\n"
	// 									"Connection: close\r\n"
	// 									"\r\n";
std::string					request = "GET /vegetables HTTP/1.1\r\n"
								"Host: localhost:8080\r\n";
								// "Connection: keep-alive\r\n"
								// "\r\n";
std::string					request2 = "Connection: keep-alive\r\n";
								// "\r\n";
								// "GET /fruits/ HTTP/1.1\r\n"
								// "Host: localhost:8080\r\n"
								// "Connection: keep-alive\r\n"
								// "\r\n"
								// "GET /vegetables/ HTTP/1.1\r\n"
								// "Host: localhost:8080\r\n"
								// "Connection: close\r\n"
								// "\r\n";
std::string					request3 = "GET / HTTP/1.1\r\n"
								"Host: localhost:5173\r\n"
								"Connection: keep-alive\r\n"
								"\r\n"
								"GET /fruits/ HTTP/1.1\r\n"
								"Host: localhost:8080\r\n"
								"Connection: keep-alive\r\n"
								"\r\n"
								"GET /fruits/ HTTP/1.1\r\n"
								"Host: localhost:8080\r\n"
								"Connection: keep-alive\r\n"
								"\r\n"
								"GET /vegetables/ HTTP/1.1\r\n"
								"Host: localhost:8080\r\n"
								"Connection: keep-alive\r\n"
								"\r\n"
								"GET /crops/ HTTP/1.1\r\n"
								"Host: localhost:8080\r\n"
								"Connection: keep-alive\r\n"
								"\r\n";
                       
										
	std::string				response;	

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((status = getaddrinfo("localhost", PORT, &hints, &clientinfo)) < 0)
	{
		std::cerr << gai_strerror(status) << std::endl;
		return (1);
	}
	for (p = clientinfo; p != NULL; p = p->ai_next)
	{
		csock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (csock  >= 0)
			break ;
	}
	if (p == NULL)
		return (1);
	if (connect(csock, (struct sockaddr *)p->ai_addr, p->ai_addrlen) < 0)
	{
		std::cerr << "Error connect: " <<std::endl;
		std::cerr << "Error: " << errno <<std::endl;
		close (csock);
		return(1);
	}
	freeaddrinfo(clientinfo);

	// char bufi[1024];
	// memset(bufi, 0, sizeof(bufi));
	//read(0, bufi, sizeof(bufi));
	// //std::cout << "fucked up" << std::endl;
	// request.clear();
	// request.append(bufi, sizeof(bufi));

	std::cerr << "Client ready to send" << std::endl;
	// if (send(csock, request.c_str(), request.size(), 0) < 0)
	// {
	// 	std::cerr << "Error: " << errno <<std::endl;
	// 	close(csock);
	// 	return(1);
	// }
	// if (send(csock, request2.c_str(), request2.size(), 0) < 0)
	// {
	// 	std::cerr << "Error: " << errno <<std::endl;
	// 	close(csock);
	// 	return(1);
	// }
	if (send(csock, request3.c_str(), request3.size(), 0) < 0)
	{
		std::cerr << "Error: " << errno <<std::endl;
		close(csock);
		return(1);
	}

	int numRequests = 10;
	for (int i = 0; i < numRequests; i ++)
	{
		std::cerr << i << "***************Client ready to receive**************" << std::endl;
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		int recvbytes = recv(csock, buf, sizeof(buf), 0);
		std::cout << "recvbytes = " << recvbytes << std::endl;
		if (recvbytes < 0)
		{
			std::cerr << "Error: " << errno <<std::endl;
			close (csock);
			return(1);
		}
		
		response.clear();
		response.append(buf, sizeof(buf));
		std::cout << "received from server: " << response << std::endl;
	}
	close (csock);
	return (0);
}


//getaddrinfo
//socket
//connect
