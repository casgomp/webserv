/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestRouting.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:15:19 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/22 14:06:31 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_ROUTING_HPP
# define REQUEST_ROUTING_HPP

# include <map>

# include "httpRequestParser.hpp"
# include "serverUtils.hpp"

void	requestRouting(int fd, std::map<int, t_client> &clients, 
			t_listenServers &listenServers, const HttpRequest &httpRequest);

#endif
