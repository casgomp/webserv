/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiExecute.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 15:45:27 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/04 15:55:00 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <sstream>
# include <string>
# include <vector>
# include <fcntl.h>
# include <unistd.h>

# include "cgiExecute.hpp"
# include "httpRequestParserUtils.hpp"
# include "requestValidationUtils.hpp"

std::vector<std::string>	buildCgiEnvp(const HttpRequest &httpRequest)
{
	std::vector<std::string>	cgiEnvp;
	std::ostringstream			oss;
	std::string					method = httpRequest.requestLine.method;

	oss << httpRequest.expectedBodyLength;
	cgiEnvp.push_back("REQUEST_METHOD=" + method);
	if (method == "GET")
		cgiEnvp.push_back("QUERY_STRING=" + httpRequest.requestLine.queryString);
	else if (method == "POST")
	{
		cgiEnvp.push_back("CONTENT_LENGTH=" + oss.str());
		//cgiEnvp.push_back("CONTENT_TYPE=" + httpRequest.requestLine.contentType);//contentType is missing from httpRequest headers in cgi POST request.
	}
	return (cgiEnvp);
}

static std::vector<char *>	toCharPtrs(const std::vector<std::string> &strs)
{
	std::vector<char *>	ptrs;

	for (size_t i = 0; i < strs.size(); i ++)
		ptrs.push_back(const_cast<char *>(strs[i].c_str()));
	ptrs.push_back(NULL);
	return (ptrs);
}

t_cgiProcess	executeCgi(const HttpRequest &httpRequest, const t_responseInstructions &responseInstructions)
{
	int				pipeR[2] = {-1, -1};
	int				pipeW[2] = {-1, -1};
	t_cgiProcess	proc;

	if (pipe(pipeR) == -1)
		;//return error?
	if (pipe(pipeW) == -1)
		;//return error?
	proc.pid = fork();
	if (proc.pid == -1)
		;//return error?
	if (proc.pid == 0)
	{
		//redirections in child, stdin and stdout
		if (pipeR[1] != -1)
			close(pipeR[1]);
		if (pipeW[0] != -1)
			close(pipeW[0]);
		dup2(pipeR[0], STDIN_FILENO);
		close(pipeR[0]);
		dup2(pipeW[1], STDOUT_FILENO);
		close(pipeW[1]);
		//execve
		std::vector<std::string>	args;
		args.push_back(responseInstructions.cgiInterpreter);
		args.push_back(responseInstructions.resolvedPath);
		std::vector<char *>	argv = toCharPtrs(args);
		std::vector<std::string>	env = buildCgiEnvp(httpRequest);
		std::vector<char *>	envp = toCharPtrs(env);

		execve(responseInstructions.cgiInterpreter.c_str(), &argv[0], &envp[0]);
		exit (1);//exit means execve didn't work
	}
	close(pipeR[0]);
	close(pipeW[1]);
	proc.stdinFd = pipeR[1];
	proc.stdoutFd = pipeW[0];
	return (proc);
}
