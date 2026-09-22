/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configInterface.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:13:40 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/22 14:31:25 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _HPP
# define _HPP

# include <string>
# include <vector>

# include "configConf.hpp"

t_httpConf		getConfigInterface(const t_block &ptreeConf);
t_serverConf	getServerConfig(const t_block &serverTreeConf, const t_httpConf &httpConf);
t_locationConf	getLocationConfig(const t_block &locationTreeConf, const t_serverConf &serverConf);

#endif
