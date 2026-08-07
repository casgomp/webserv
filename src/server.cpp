#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "3490"
#include <netinet/in.h>
#include <arpa/inet.h>

int	server()
{
	int						status;
	struct addrinfo			hints;
	struct addrinfo 		*servinfo;
	int						servsock;
	int						backlog = 5; //max length of pending connections queue.
	int						clientsock;
	struct sockaddr_storage	client_addr;
	socklen_t				addr_size;
	//send(), recv() stuff below:
	int						len;
	int						bytes_sent;
	char msg[] = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 12\r\n"
    "\r\n"
    "Hello World!";
	int						bytes_received;
	char					buf[1024];
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
	{
		std::cerr << gai_strerror(status) << std::endl;
		return(1);
	}
	servsock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (servsock == -1)
	{
		freeaddrinfo(servinfo);
		return (1);
	}
	int yes = 1;
	setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	//the type argument serves a second purpose: in addition to specifying
	//a socket type, it may include the bitwise OR of any of any of the 
	//following values, to modify the behavior of socket(): SOCK_NONBLOCK, etc.
	//fcntl can also be used to change the status of the socket to SOCK_NONBLOCK
	//use fcntl since it works fine in single threaded servers and is portable,
	//the other way solves the race condition issue for multithreaded servers, but is
	//not entirely portable. But this server is single threaded so not necessary.
	
	//fcntl(servsock, F_SETFL, O_NONBLOCK);
	
	//should loop through all results in the servinfo struct and bind to the 
	//first possibile one: for (p = servinfo; p != NULL; p = p->ai_next)
	//setsockopt() before binding:
	//if bind fails because "address already in use", you can use the
	//setsockopt to allow the program to reuse the port.

	// std::cout << "IP addresses for localhost 3409: \n";
	// struct addrinfo *p;
	// char ipstr[INET6_ADDRSTRLEN];
	// for (p = servinfo; p != NULL; p = p->ai_next)
	// {
	// 	void *addr;
	// 	int ipver;
	// 	struct sockaddr_in *ipv4;
	// 	struct sockaddr_in6 *ipv6;

	// 	// get the pointer to the address itself,
	// 	// different fields in IPv4 and IPv6:
	// 	if (p->ai_family == AF_INET) { // IPv4
	// 		ipv4 = (struct sockaddr_in *)p->ai_addr;
	// 		addr = &(ipv4->sin_addr);
	// 		ipver = 4;
	// 	} else { // IPv6
	// 		ipv6 = (struct sockaddr_in6 *)p->ai_addr;
	// 		addr = &(ipv6->sin6_addr);
	// 		ipver = 6;
	// 	}
	// 	// convert the IP to a string and print it:
	// 	inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
	// 	std::cout << ipver << ": " << ipstr << "\n"; 
	// }
	if (bind(servsock, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		freeaddrinfo(servinfo);
		close(servsock);
		return (1);
	}
	freeaddrinfo(servinfo);//this should at every error 1 return above.
	if (listen(servsock, backlog) == -1)
	{
		close(servsock);
		return (1);
	}
	addr_size = sizeof(client_addr);
	

	clientsock = accept(servsock, (struct sockaddr *)&client_addr, &addr_size);
	if (clientsock == -1)
	{
		close(servsock);
		return (1);
	}
	
	
	len = strlen(msg);
	bytes_sent = send(clientsock, msg, len, 0);
	if (bytes_sent == -1)
	{
		close(servsock);
		close(clientsock);
		return (1);
	}
	len = strlen(buf);
	bytes_received = recv(clientsock, buf, sizeof(buf), 0);
	std::cout << "n bytes received from client = " << bytes_received << std::endl;
	for (int i = 0; i < bytes_received; i++)
		std::cout << buf[i];


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