/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiExecute.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 15:47:37 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/10 17:29:07 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_EXECUTE_HPP
# define CGI_EXECUTE_HPP

# include <vector>
# include <string>

# include "httpRequestParserUtils.hpp"
# include "requestValidationUtils.hpp"

typedef struct s_cgiProcess {
	pid_t		pid;
	int			stdinFd;
	int			stdoutFd;
	std::string	body;
	std::string	output;
	size_t		bytesSent;
	s_cgiProcess() : pid(-1), stdinFd(-1), stdoutFd(-1) {}
} t_cgiProcess;

typedef struct	s_cgiOutput {
	bool		success;
	std::string	buffer;
	s_cgiOutput() : success (false) {}
} t_cgiOutput;

int							executeCgi(const HttpRequest &httpRequest, const t_responseInstructions &responseInstructions, t_cgiProcess &cgiProcess);
std::vector<std::string>	buildCgiEnvp(HttpRequest &httpRequest);

#endif
