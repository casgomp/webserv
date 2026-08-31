#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <netdb.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

//server macros?

//error message macros? like the ones in closeConnection()?

//directive blocks (parsing)
typedef struct	s_block {
	std::vector<std::pair<std::string, std::string> >						directives;
	std::vector<std::pair<std::pair<std::string, std::string>, s_block> >	children;
} t_block;

typedef struct	s_location {
	std::string							root; //inherit
	size_t								clientMaxBodySize; //inherit
	bool								autoindex; //inherit
	std::vector<std::string>			allowedMethods;
	s_location() : clientMaxBodySize(0), autoindex(false)
	{
		allowedMethods.push_back("GET");
		allowedMethods.push_back("POST");
		allowedMethods.push_back("DELETE");
	}
} t_location;

typedef struct	s_server {
	std::string											root; //inherit
	size_t												clientMaxBodySize; //inherit
	bool												autoindex; //inherit
	std::vector<std::string>							serverNames;
	std::vector<t_location>								locations;
	std::vector<std::pair<std::string, std::string> >	listen;
	std::map<int, std::string>							errorPages;
	s_server() : clientMaxBodySize(0), autoindex(false) {}
} t_server;

typedef struct	s_httpConf {
	std::string							root; //inherit
	size_t								clientMaxBodySize; //inherit
	bool								autoindex; //inherit
	std::vector<t_server>				servers;
	s_httpConf() : clientMaxBodySize(0), autoindex(false) {}
} t_httpConf;

//listen pairs map

//server events
typedef struct	s_client {
	int			fd;
	//int		port;
	std::string	request;
	std::string	response;
	size_t		bytes_sent;
} t_client;

//main
int										main(int argc, char **argv);
//server -->divide in init and events
int										server();
void									closeConnection(int fd, std::map<int, t_client> &clients, int flag_err);
void									cleanupServ(int servsock, int epfd, std::map<int, t_client> &clients, int flag_err);
//configParse
t_block									parseConfig(const char *filename);//return type should change so vectorcontains t_blocks?
int										readConfigToString(const char *filename, std::string &str);
t_block									recurseConfig(const std::string &str);
std::pair<std::string, std:: string>	splitter(const std::string &chunk);
//configInterface
t_httpConf								getConfigInterface(t_block ptreeConf);


#endif