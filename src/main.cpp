#include "../include/webserv.hpp"

int	main(int argc, char **argv)
{
	t_block	conf;

	if (argc == 1)
		conf = processConfigFile("config/minimal.conf");
	else if (argc == 2)
		conf = processConfigFile(argv[1]);
	else
	{
		std::cerr << "./webserv <file.conf>" << std::endl;
		return (1);
	}

	return (0);//for now to test parsing only***************


	//if conf is invalid
		//return (1);
	if (server() == 1) //pass config struct/object to server
		return (1);
	return(0);
}