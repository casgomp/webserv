/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiTypes.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 16:06:16 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/28 21:12:52 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_TYPES_HPP
# define CGI_TYPES_HPP

# include <string>
# include <sys/types.h>
# include <ctime>

typedef struct s_cgiProcess {
	pid_t		pid;
	std::time_t	startTime;
	int			stdinFd;
	int			stdoutFd;
	std::string	body;
	std::string	output;
	size_t		bytesSent;
	s_cgiProcess() : pid(-1), startTime(0), stdinFd(-1), stdoutFd(-1), bytesSent(0) {}
} t_cgiProcess;

#endif
