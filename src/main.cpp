#include "../include/webserv.hpp"

int	main(int argc, char **argv)
{
	t_block		pTreeConf;
	t_httpConf	httpConf;

	if (argc == 1)
		pTreeConf = parseConfig("config/minimal.conf");
	else if (argc == 2)
		pTreeConf = parseConfig(argv[1]);
	else
	{
		std::cerr << "./webserv <file.conf>" << std::endl;
		return (1);
	}
	
	printConfig(pTreeConf);
	std::cout << "pTreeConf.children[0].first.first = " << pTreeConf.children[0].first.first << std::endl;
	
	httpConf = getConfigInterface(pTreeConf);//process config file returns a conf
	if (httpConf.level.empty())
		return(1);
	
	
	
	
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