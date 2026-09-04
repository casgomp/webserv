/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverInit.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 14:56:19 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/04 17:23:40 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"











std::map<std::pair<std::string, std::string>, std::vector<t_serverConf *> >	getListeningServers(t_httpConf &httpConf)
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