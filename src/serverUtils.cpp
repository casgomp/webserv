/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 14:58:17 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/17 15:46:26 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <sys/epoll.h>
# include <unistd.h>
# include <errno.h>

# include "serverUtils.hpp"
# include "serverInit.hpp"
# include "serverClean.hpp"
# include "requestValidationUtils.hpp"

int	epollSet(int epfd, int operation, int fd, int events)
{
	struct epoll_event	ev;

	ev.events = events;
	ev.data.fd = fd;
	return (epoll_ctl(epfd, operation, fd, &ev));
}

size_t	computeCeilingBody(const t_listenServers &listenServers)
{
	size_t	ceilingClientMaxBodySize = 0;

	for(t_listenServers::const_iterator it = listenServers.begin(); it != listenServers.end(); it ++)
	{
		for (size_t i = 0; i < it->second.size(); i ++)
		{
			if (it->second[i]->ceilingClientMaxBodySize > ceilingClientMaxBodySize)
				ceilingClientMaxBodySize = it->second[i]->ceilingClientMaxBodySize;
		}
	}
	return (ceilingClientMaxBodySize);
}

t_responseInstructions	buildErrorInstructions(const int &status, t_serverConf *serverConf)
{
	t_responseInstructions						instructions;
	std::map<int,std::string>::const_iterator	it;
	
	instructions.statusCode = status;
	it = serverConf->errorPages.find(status);
	if (it != serverConf->errorPages.end())
		instructions.resolvedPath = it->second;
	else
		instructions.resolvedPath = "";
	return (instructions);
}