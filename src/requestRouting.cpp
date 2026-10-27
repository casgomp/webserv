/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestRouting.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 14:24:17 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/27 16:36:52 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <algorithm>
# include <map>
# include <stdexcept>
# include <string>
# include <vector>
# include <cctype>

# include "requestRouting.hpp"
# include "httpRequestParser.hpp"
# include "serverInit.hpp"
# include "serverUtils.hpp"


static std::string	toLowerCopy(std::string s)
{
	for (size_t i = 0; i < s.size(); i++)
		s[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(s[i])));
	return (s);
}

void	requestRouting(int fd, std::map<int, t_client> &clients,
			t_listenServers &listenServers, const HttpRequest &httpRequest)
{
	std::vector<t_serverConf *>	confServers = listenServers[clients[fd].pairAddressPort];
	t_serverConf				*selected;

	if (confServers.empty())
		throw std::runtime_error("internal error: no servers found for listening pair");
	selected = confServers[0];
	if (confServers.size() > 1 && httpRequest.headers.count("host"))
	{
		std::string	host = httpRequest.headers.at("host");
		bool		found = false;

		host = toLowerCopy(host.substr(0, host.find(':')));
		for (size_t i = 0; i < confServers.size() && !found; i++)
		{
			const std::vector<std::string>	&names = confServers[i]->serverNames;

			for (size_t j = 0; j < names.size() && !found; j++)
			{
				if (toLowerCopy(names[j]) == host)
				{
					selected = confServers[i];
					found = true;
				}
			}
		}
	}
	clients[fd].serverConf = selected;
}