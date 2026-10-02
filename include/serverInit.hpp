/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverInit.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:14:53 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/22 13:40:24 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_INIT_HPP
# define SERVER_INIT_HPP

# include <map>
# include <string>
# include <vector>

# include "configConf.hpp"

typedef std::map<std::pair<std::string, std::string>, std::vector<t_serverConf *> >	t_listenServers;
typedef std::map<int, std::pair<std::string, std::string> >							t_listeningSockets;

t_listenServers		getListenServers(t_httpConf &httpConf);
t_listeningSockets	serverInit(t_listenServers &listenServers);

#endif
