/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverEvent.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:14:42 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/09 15:37:29 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_EVENT_HPP
# define SERVER_EVENT_HPP

# include "serverInit.hpp"

#define MAX_EVENTS 64
#define BUFFER_SIZE 1024

void	serverEvent(t_listenServers &listenServers, t_listeningSockets &listeningSockets);

#endif
