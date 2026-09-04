/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverInit.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 14:56:19 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/04 19:28:17 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"


t_listeningSockets	serverInit(const t_listenServers &listenServers)
{
	//listening socket: getaddrinfo(), socket(), bind(), listen();
	int													status;
	struct addrinfo										hints;
	struct addrinfo										*servinfo;
	struct addrinfo										*p;
	// int													servsock = -1;//change name to listening socket maybe should be a map<int, pair<str,str> >? to easily find based on fd.
	std::map<int, std::pair<std::string, std::string> >	listeningSockets; // make typedef
	int													yes;
	int													backlog = 32;
	struct sockaddr_storage								client_addr;
	socklen_t											addr_size;
	
	for (t_listenServers::iterator	it = listenServers.begin(); it != listenServers.end(); it ++)
	{
		//for loop for every key in the map of address:port pairs
		//set separate const char* vars for address and port....if any are empty, set to NULL
		//pass as arguments to getaddrinfo
		//enter nested for loop 
			//when socket() is called, store the fd in some tmp int (currently servsock)
		//end of for loop, enter the fd number in the listeningSockets key, and with its address and port pair as value
	}

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
}



t_listenServers	getListenServers(t_httpConf &httpConf)
{
	std::pair<std::string, std::string>													pairPortAddress;
	std::map<std::pair<std::string, std::string>, std::vector<t_serverConf *> >			listenServers;

	for (size_t i = 0; i < httpConf.servers.size(); i ++)
	{
		for (size_t j = 0; j < httpConf.servers[i].listen.size(); j ++)
		{
			if (httpConf.servers[i].listen[j].first == "*")
				pairPortAddress.first = "";
			else
				pairPortAddress.first = httpConf.servers[i].listen[j].first;
			pairPortAddress.second = httpConf.servers[i].listen[j].second;
			// std::cout << "Server: " << httpConf.servers[i].serverNames.at(0) << " @" << pairPortAddress.first << ":" << pairPortAddress.second << std::endl;
			listenServers[pairPortAddress].push_back(&httpConf.servers[i]);
		}
	}
	return (listenServers);
}