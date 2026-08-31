#include "../include/webserv.hpp"

int	main(int argc, char **argv)
{
	t_block		configParseTree;
	t_httpConf	httpConf;

	if (argc == 1)
		configParseTree = parseConfig("config/minimal.conf");
	else if (argc == 2)
		configParseTree = parseConfig(argv[1]);
	else
	{
		std::cerr << "./webserv <file.conf>" << std::endl;
		return (1);
	}

	return (0);//for now to test parsing only***************

	httpConf = getConfigInterface(configParseTree);//process config file returns a conf
	
	//get the ports return map of all ports that will listen
	//serverInit (ports, conf?)
	//serverEvents (ports, conf?)

	//if conf is invalid
		//return (1);
	if (server() == 1) //pass config struct/object to server
		return (1);
	return(0);
}