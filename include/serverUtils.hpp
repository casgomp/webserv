/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverUtils.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:15:04 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/10 17:31:29 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_UTILS_HPP
# define SERVER_UTILS_HPP

// # include <utility>
# include <string>

# include "configConf.hpp"
# include "configConf.hpp"
# include "cgiExecute.hpp"
# include "serverInit.hpp"

typedef struct	s_client {
	std::pair<std::string, std::string>	pairAddressPort;
	t_serverConf						*serverConf;
	t_cgiProcess						cgiProcess;
	std::string							request;
	std::string							response;
	size_t								bytesSent;
	bool								keepAlive;
} t_client;

int		epollSet(int epfd, int operation, int fd, int events);
size_t	computeCeilingBody(const t_listenServers &listenServers);
#endif