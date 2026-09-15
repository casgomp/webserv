/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestRouting.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 14:24:17 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/14 17:35:08 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

void	requestRouting(int fd, std::map<int, t_client> &clients, 
			t_listenServers &listenServers, const HttpRequest &httpRequest)
{
	std::vector<t_serverConf *> 		confServers = listenServers[clients[fd].pairAddressPort];
	t_serverConf						*selected;

	if (confServers.empty())
	{
		;//set httpRequest to 500 (Internal Server Error)
		return ;
	}
	
	selected = confServers[0];
	if (confServers.size() > 1 && httpRequest.headers.count("host"))
	{
		const std::string &host = httpRequest.headers.at("host");
		for (size_t i = 0; i < confServers.size(); i ++)
		{
			const std::vector<std::string> &names = confServers[i]->serverNames;
			if (std::find(names.begin(), names.end(), host) != names.end())
			{
				selected = confServers[i];
				break ;
			}
		}
	}
	clients[fd].serverConf = selected;
}