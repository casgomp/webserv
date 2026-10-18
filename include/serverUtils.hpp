/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverUtils.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:15:04 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/18 15:15:24 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_UTILS_HPP
# define SERVER_UTILS_HPP

// # include <utility>
# include <string>

# include "configConf.hpp"
# include "configConf.hpp"
# include "serverInit.hpp"
# include "cgiTypes.hpp"

typedef struct	s_cgi {
	bool		success;
	std::string	interpreter;
	std::string	queryString;
	time_t		timeout;
	s_cgi() : success (false), interpreter(""), queryString("") {}
} t_cgi;

typedef struct	s_responseInstructions {
	int			statusCode;
	bool		isRedirect;
	std::string	redirectLocation;
	bool		isCgi;
	t_cgi		cgi;
	bool		isAutoIndex;
	std::string	resolvedPath;
	std::string	contentType;
	std::string	method;
	std::string body;
	bool		keepAlive;
	s_responseInstructions() : statusCode(0), isRedirect(false), redirectLocation(""), isCgi(false), 
		isAutoIndex(false), resolvedPath(""), contentType(""), keepAlive(false) {}
} t_responseInstructions;

typedef struct	s_client {
	std::pair<std::string, std::string>	pairAddressPort;
	t_serverConf						*serverConf;
	t_cgiProcess						cgiProcess;
	std::string							request;
	std::string							response;
	t_responseInstructions				responseInstructions;
	size_t								bytesSent;
	time_t								lastActivity;
	bool								waitingForClient;
	s_client() : serverConf(NULL) {}
} t_client;

typedef struct	s_serverState {
	int						epfd;
	std::map<int, t_client>	clients;
	std::map<int, int>		fdPipeToClient;
	s_serverState() : epfd (-1) {}
} t_serverState;

int						epollSet(int epfd, int operation, int fd, int events);
size_t					computeCeilingBody(const t_listenServers &listenServers);
t_responseInstructions	buildErrorInstructions(const int &status, t_serverConf *serverConf);
void					resetInstructions(t_client &client, const t_responseInstructions &newInstructions);
void					resetErrorInstructions(t_serverState &ctx, const int &fd, const int &status);

#endif