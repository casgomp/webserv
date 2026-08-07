#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int server()
{
	struct pollfd	pfds[1];
	int				num_events;
	int				pollin_happened;
	
	pfds[0].fd = 0;
	pfds[0].events = POLLIN;

	std::cout << "Hit RETURN or wait 10 seconds for timeout" << std::endl;
	num_events = poll(pfds, 1, 10000);//10 second timeout
	if (num_events == 0)
		std::cout << "Poll timed out" << std::endl;
	else
	{
		pollin_happened = pfds[0].revents & POLLIN;
		if (pollin_happened)
		{
			char buf[256];
			int bytes_read = read(0, buf, sizeof(buf));
			std::cout << "File descriptor " << pfds[0].fd << " is ready to read" << std::endl;
			write(1, buf, bytes_read);
		}
		else
			std::cout << "Unexpected event occured: " << pfds[0].revents << std::endl;
	}
	return (0);
}