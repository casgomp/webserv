/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverInit.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 14:56:19 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/07 13:51:38 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

t_listeningSockets	serverInit(t_listenServers &listenServers)
{
	const char											*address;
	const char											*port;
	int													status;
	struct addrinfo										hints;
	struct addrinfo										*servinfo;
	struct addrinfo										*p;
	int													fdServer = -1;
	int													yes;
	int													backlog = 32;
	t_listeningSockets									listeningSockets;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	for (t_listenServers::iterator it = listenServers.begin(); it != listenServers.end(); it ++)//loop for every key in address:port pairs map
	{
		if (it->first.first == "")
			address = NULL;
		else
			address = it->first.first.c_str();
		if (it->first.second == "")
			port = NULL;
		else
			port = it->first.second.c_str();
		if ((status = getaddrinfo(address, port, &hints, &servinfo)) != 0)
			throw std::runtime_error(gai_strerror(status));
		for (p = servinfo; p != NULL; p = p ->ai_next)
		{
			fdServer = socket(p->ai_family, p->ai_socktype | SOCK_NONBLOCK, p->ai_protocol);
			if (fdServer < 0)
			{
				// std::cout << "socket" << std::endl;//DEBUGGING
				continue;
			}
			yes = 1;
			if (setsockopt(fdServer, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
			{
				close(fdServer);
				// std::cout << "setsockopt" << std::endl;//DEBUGGING
				continue ;
			}
			if (bind(fdServer, p->ai_addr, p->ai_addrlen) < 0)
			{
				close(fdServer);
				// std::cout << "bind " << address << ":" << port << std::endl;//DEBUGGING
				continue ;
			}
			if (listen(fdServer, backlog) < 0)
			{
				close(fdServer);
				// std::cout << "listen" << std::endl;//DEBUGGING
				continue ;
			}
			break ;
		}
		freeaddrinfo(servinfo);
		if (p == NULL)
			throw std::runtime_error(strerror(errno));
		listeningSockets[fdServer] = std::make_pair(it->first.first, it->first.second);
		// std::cout << "listeningSockets[" << fdServer << "] = " << listeningSockets[fdServer].first << ":" << listeningSockets[fdServer].second << std::endl;
	}
	return (listeningSockets);
}

t_listenServers	getListenServers(t_httpConf &httpConf)
{
	std::pair<std::string, std::string>	pairPortAddress;
	t_listenServers						listenServers;

	for (size_t i = 0; i < httpConf.servers.size(); i ++)
	{
		for (size_t j = 0; j < httpConf.servers[i].listen.size(); j ++)
		{
			if (httpConf.servers[i].listen[j].first == "*")
				pairPortAddress.first = "";
			else
				pairPortAddress.first = httpConf.servers[i].listen[j].first;
			pairPortAddress.second = httpConf.servers[i].listen[j].second;
			listenServers[pairPortAddress].push_back(&httpConf.servers[i]);
		}
	}
	return (listenServers);
}