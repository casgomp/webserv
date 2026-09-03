/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42berlin.d      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 18:26:29 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/03 18:26:36 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

int	main(int argc, char **argv)
{
	t_block		pTreeConf;
	t_httpConf	httpConf;

	try
	{
		if (argc == 1)
			pTreeConf = parseConfig("config/minimal.conf");
		else if (argc == 2)
			pTreeConf = parseConfig(argv[1]);
		else
		{
			std::cerr << "./webserv <file.conf>" << std::endl;
			return (1);
		}
		// printConfig(pTreeConf);
		httpConf = getConfigInterface(pTreeConf);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Config Error: " << e.what() << std::endl;
		return (1);
	}
	
	
	
	
	return (0);//for now to test parsing only***************


	//get the ports return map of all ports that will listen
	//serverInit (ports, conf?)
	//serverEvents (ports, conf?)

	//if conf is invalid
		//return (1);
	if (server() == 1) //pass config struct/object to server
		return (1);
	return(0);
}
