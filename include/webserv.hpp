#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <iostream>
#include <map>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

//config parser
#include <vector>

//directive blocks
typedef struct	s_block {
	std::vector<std::pair<std::string, std::string> >	directives;
	// std::vector<std::pair<std::string, s_block> >		children;
	std::vector<std::pair<std::pair<std::string, std::string>, s_block> >		children;
} t_block;

//configparser
typedef struct	s_servconf {
	int	port;
} t_servconf;

//server
typedef struct	s_client {
	int			fd;
	std::string	request;
	std::string	response;
	size_t		bytes_sent;
} t_client;

//main
int										main(int argc, char **argv);
//server
int										server();
void									closeConnection(int fd, std::map<int, t_client> &clients, int flag_err);
void									cleanupServ(int servsock, int epfd, std::map<int, t_client> &clients, int flag_err);
//config_parser
t_block									processConfigFile(const char *filename);//return type should change so vectorcontains t_blocks?
int										readConfigFileToString(const char *filename, std::string &str);
t_block									parseConfigFile(const std::string &str);
std::pair<std::string, std:: string>	splitter(const std::string &chunk);

#endif