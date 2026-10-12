/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 14:58:17 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/10 17:14:15 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <sys/epoll.h>
# include <unistd.h>
# include <errno.h>

# include "serverUtils.hpp"
# include "serverInit.hpp"
# include "serverClean.hpp"

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