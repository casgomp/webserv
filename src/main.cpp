#include "../include/webserv.hpp"

int	main(int argc, char **argv)
{
	t_block	conf;

	if (argc != 2)
	{
		std::cerr << "./webserv <file.conf>" << std::endl;
		return (1);
	}
	conf = processConfigFile(argv[1]);

	return (0);//for now to test parsing only
	//if conf is invalid
		//return (1);
	if (server() == 1) //pass config struct/object to server
		return (1);
	return(0);
}