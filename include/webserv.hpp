/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 18:26:55 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/13 17:49:05 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <algorithm>
# include <cctype>
# include <cstring>
# include <dirent.h>
# include <errno.h>
# include <fcntl.h>
# include <fstream>
# include <iostream>
# include <map>
# include <netdb.h>
# include <stdlib.h>
# include <string>
# include <string.h>
# include <sstream>
# include <sys/epoll.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>
# include <vector>

# include "httpRequestParser.hpp"

//config macros
# define FALLBACK_ROOT "content/"
# define FALLBACK_CLIENT_MAX_BODY_SIZE 1048576
# define FALLBACK_AUTOINDEX false

//server macros
// #define PORT "3490" //not needed anymore
#define MAX_EVENTS 64
#define BUFFER_SIZE 1024

//error message macros? like the ones in closeConnection()?

//directive blocks (parsing)
typedef struct	s_block {
	std::vector<std::pair<std::string, std::string> >						directives;
	std::vector<std::pair<std::pair<std::string, std::string>, s_block> >	children;
} t_block;

//config
typedef struct	s_locationConf {
	std::string							root; //inherit
	size_t								clientMaxBodySize; //inherit
	bool								autoindex; //inherit
	std::vector<std::string>			index; //inherit
	std::string							path;
	std::vector<std::string>			allowedMethods;//nicer would be a map<std::string, bool>
	std::pair<int, std::string>			redirection;
	s_locationConf() : clientMaxBodySize(0), autoindex(false)
	{
		index.push_back("index.html");
		allowedMethods.push_back("GET");
		allowedMethods.push_back("POST");
		allowedMethods.push_back("DELETE");
	}
} t_locationConf;

typedef struct	s_serverConf {
	std::string											root; //inherit
	size_t												clientMaxBodySize; //inherit
	bool												autoindex; //inherit
	std::vector<std::string>							index; //inherit
	std::vector<std::string>							serverNames;
	std::vector<std::pair<std::string, std::string> >	listen;
	std::map<int, std::string>							errorPages;
	std::vector<t_locationConf>							locations;
	s_serverConf() : clientMaxBodySize(0), autoindex(false)
	{
		index.push_back("index.html");
	}
} t_serverConf;

typedef struct	s_httpConf {
	std::string							root; //inherit
	size_t								clientMaxBodySize; //inherit
	// Sets the maximum allowed size of the client request body. 
	// If the size in a request exceeds the configured value, the 413 
	// (Request Entity Too Large) error is returned to the client. 
	// Please be aware that browsers cannot correctly display this error. 
	// Setting size to 0 disables checking of client request body size. 
	bool								autoindex; //inherit
	std::vector<std::string>			index;
	std::vector<t_serverConf>			servers;
	s_httpConf() : clientMaxBodySize(0), autoindex(false)
	{
		index.push_back("index.html");
	}
} t_httpConf;

//server init
typedef std::map<std::pair<std::string, std::string>, std::vector<t_serverConf *> >	t_listenServers;
typedef std::map<int, std::pair<std::string, std::string> >							t_listeningSockets;

//server events
typedef struct	s_client {
	// int									fd;//not needed?
	std::pair<std::string, std::string>	pairAddressPort;
	t_serverConf						*serverConf;
	std::string							request;
	std::string							response;
	size_t								bytes_sent;
} t_client;

//main
int										main(int argc, char **argv);
//server -->divide in init and events
// int										server();
void									closeClientConnection(int fd, std::map<int, t_client> &clients, int flag_err);
void									cleanupServ(t_listeningSockets &listeningSockets, int epfd, std::map<int, t_client> &clients);
void									closeListeningSockets(t_listeningSockets &listeningSockets);
//configParse
t_block									parseConfig(const char *filename);
void									readConfigToString(const char *filename, std::string &str);
t_block									recurseConfig(const std::string &str);
std::pair<std::string, std:: string>	splitter(const std::string &chunk);
void									printConfig(t_block conf, int depth = 0);
//configInterface
t_httpConf								getConfigInterface(const t_block &ptreeConf);
t_serverConf							getServerConfig(const t_block &serverTreeConf, const t_httpConf &httpConf);
t_locationConf							getLocationConfig(const t_block &locationTreeConf, const t_serverConf &serverConf);
//configInterfaceUtils
void									checkIfValidDir(const std::string &path);
int										strToNum(const std::string &str);
int										checkAutoindex(const std::string &autoindex);
std::vector<std::string>				checkIndexFiles(const std::string &input);
void									addServerNames(t_serverConf &serverConf, const std::string &input);
void									addListenAddressPort(t_serverConf &serverConf, const std::string &input);
void									addErrorPages(t_serverConf &serverConf, const std::string &input);
void									addAllowedMethods(t_locationConf &locationConf, const std::string &input);
void									addRedirection(t_locationConf &locationConf, const std::string &input);
//serverInit
t_listenServers							getListenServers(t_httpConf &httpConf);
t_listeningSockets						serverInit(t_listenServers &listenServers);
//serverEvent
void									serverEvent(t_listenServers &listenServers, t_listeningSockets &listeningSockets);


#endif
