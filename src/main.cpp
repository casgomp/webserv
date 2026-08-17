#include "../include/webserv.hpp"

int	main(int argc, char **argv)
{
	std::vector<t_servconf>	conf;

	if (argc != 2)
	{
		std::cerr << "./webserv <file.conf>" << std::endl;
		return (1);
	}
	conf = configParser(argv[1]);
	if (conf.empty())
		return (1);
	// call config parser function: configstruct = configparser()
	// if parsing fails return (1);
	if (server() == 1) //pass config struct/object to server
		return (1);
	return(0);
}