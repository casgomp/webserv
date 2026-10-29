/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 18:26:29 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/28 16:54:18 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <iostream>

# include "main.hpp"
# include "configParse.hpp"
# include "configConf.hpp"
# include "configInterface.hpp"
# include "serverInit.hpp"
# include "serverEvent.hpp"


int	main(int argc, char **argv)
{
	t_block				pTreeConf;
	t_httpConf			httpConf;
	t_listenServers		listenServers;
	t_listeningSockets	listeningSockets;

	try
	{
		if (argc == 1)
			pTreeConf = parseConfig("config/default.conf");
		else if (argc == 2)
			pTreeConf = parseConfig(argv[1]);
		else
		{
			std::cerr << "./webserv <file.conf>" << std::endl;
			return (1);
		}
		httpConf = getConfigInterface(pTreeConf);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Config Error: " << e.what() << std::endl;
		return (1);
	}
	try
	{
		listenServers = getListenServers(httpConf);
		
		listeningSockets = serverInit(listenServers);
	}
	catch (const std::exception &e)
	{
		std::cerr  << "Server Init Error: " << e.what() << std::endl;
		return (1);
	}
	try
	{
		serverEvent(listenServers, listeningSockets);
	}
	catch (const std::exception &e)
	{
		std::cerr  << "Server Event Error: " << e.what() << std::endl;
		return (1);
	}
	return(0);
}
