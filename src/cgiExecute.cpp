/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiExecute.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 15:45:27 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/24 16:12:12 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <ctime>
# include <sstream>
# include <string>
# include <vector>
# include <fcntl.h>
# include <unistd.h>

# include "cgiExecute.hpp"
# include "httpRequestParserUtils.hpp"
# include "requestValidationUtils.hpp"
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
		//if httpRequest.headers.find("content-type") == httpRequest.headers.end()
		//cgiEnvp.push_back("CONTENT_TYPE=" + httpRequest.headers[contentType]);//contentType is missing from httpRequest headers in cgi POST request.
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

int	fcntlSet(int pipeFd, int *pipeR, int *pipeW, int cmd, int arg)
{
	if (fcntl(pipeFd, cmd, arg) < 0)
	{
		close (pipeR[0]);
		close (pipeR[1]);
		close (pipeW[0]);
		close (pipeW[1]);
		return (-1);
	}
	return (0);
}

int	executeCgi(const HttpRequest &httpRequest, t_serverState &ctx, const int &fd)
{
	int				pipeR[2] = {-1, -1};
	int				pipeW[2] = {-1, -1};

	if (pipe(pipeR) == -1)
		return (-1);
	if (pipe(pipeW) == -1)
	{
		close (pipeR[0]);
		close (pipeR[1]);
		return (-1);
	}
	if (fcntlSet(pipeR[0], pipeR, pipeW, F_SETFD, FD_CLOEXEC) < 0)
		return (-1);
	if (fcntlSet(pipeR[1], pipeR, pipeW, F_SETFD, FD_CLOEXEC) < 0)
		return (-1);
	if (fcntlSet(pipeW[0], pipeR, pipeW, F_SETFD, FD_CLOEXEC) < 0)
		return (-1);
	if (fcntlSet(pipeW[1], pipeR, pipeW, F_SETFD, FD_CLOEXEC) < 0)
		return (-1);
	ctx.clients[fd].cgiProcess.pid = fork();
	if (ctx.clients[fd].cgiProcess.pid == -1)
	{
		close (pipeR[0]);
		close (pipeR[1]);
		close (pipeW[0]);
		close (pipeW[1]);
		return (-1);
	}
	if (ctx.clients[fd].cgiProcess.pid > 0)
    	ctx.clients[fd].cgiProcess.startTime = time(NULL);
	if (ctx.clients[fd].cgiProcess.pid == 0)
	{
		if (pipeR[1] != -1)
			close(pipeR[1]);
		if (pipeW[0] != -1)
			close(pipeW[0]);
		dup2(pipeR[0], STDIN_FILENO);
		close(pipeR[0]);
		dup2(pipeW[1], STDOUT_FILENO);
		close(pipeW[1]);
		std::vector<std::string>	args;
		args.push_back(ctx.clients[fd].responseInstructions.cgi.interpreter);
		args.push_back(ctx.clients[fd].responseInstructions.resolvedPath);
		std::vector<char *>	argv = toCharPtrs(args);
		std::vector<std::string>	env = buildCgiEnvp(httpRequest);
		std::vector<char *>	envp = toCharPtrs(env);
		execve(ctx.clients[fd].responseInstructions.cgi.interpreter.c_str(), &argv[0], &envp[0]);
		_exit (127);
	}
	close(pipeR[0]);
	close(pipeW[1]);
	if (httpRequest.requestLine.method == "GET")
	{
		close (pipeR[1]);
		pipeR[1] = -1;
	}
	ctx.clients[fd].cgiProcess.stdinFd = pipeR[1];
	ctx.clients[fd].cgiProcess.stdoutFd = pipeW[0];
	return (0);
}
