#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <iostream>
#include <map>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef struct	Client {
	int			fd;
	std::string	request;
	std::string	response;
	size_t		bytes_sent;
} t_client;

int		main();
int		server();
void	closeConnection(int fd, std::map<int, t_client> &clients, int flag_err);
void	cleanupServ(int servsock, int epfd, std::map<int, t_client> &clients, int flag_err);

#endif