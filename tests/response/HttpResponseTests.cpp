/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseTests.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 11:36:28 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/01 23:11:42 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponseTests.hpp"
#include "../../include/httpResponse.hpp"
#include <fstream>
#include <cstdio>

HttpResponseTests::HttpResponseTests() : TestSuite("HttpResponseTests") {}

void HttpResponseTests::run_all()
{
	std::cout << "\n\033[30;105mRunning HttpResponseTests...\033[0m\n" << std::endl;

	validateResponseLine();
	testAutoIndex();
	testPost();
	testDelete();
	printSummary();
}

void	HttpResponseTests::validateResponseLine()
{
	t_responseInstructions instructions;
	instructions.statusCode = 404;
	instructions.contentType = "text/html";
	instructions.closeConnection = true;
	std::string result = buildHttpResponse(instructions, "", "GET");
	std::string response = "HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 0\r\n"
		"Connection: close\r\n\r\n";
	check(response == result, "build response line for 404");
	std::cout << "Result: " << result << std::endl;

	instructions.statusCode = 200;
	instructions.contentType = "text/html";
	instructions.closeConnection = false;
	result = buildHttpResponse(instructions, "", "GET");
	response = "HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 0\r\n"
		"Connection: keep-alive\r\n\r\n";
	check(response == result, "build response line for 200");
	std::cout << "Result: " << result << std::endl;

	instructions.statusCode = 308;
	instructions.contentType = "text/html";
	instructions.closeConnection = false;
	instructions.isRedirect = true;
	instructions.redirectLocation = "/fruits";
	result = buildHttpResponse(instructions, "", "GET");
	response = "HTTP/1.1 308 Permanent Redirect\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 0\r\n"
		"Location: /fruits\r\n"
		"Connection: keep-alive\r\n\r\n";
	check(response == result, "build response line for 308");
	std::cout << "Result: " << result << std::endl;

	instructions.statusCode = 308;
	instructions.contentType = "text/html";
	instructions.closeConnection = false;
	instructions.isRedirect = true;
	instructions.redirectLocation = "/vegetables";
	result = buildHttpResponse(instructions, "", "GET");
	response = "HTTP/1.1 308 Permanent Redirect\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 0\r\n"
		"Location: /vegetables\r\n"
		"Connection: keep-alive\r\n\r\n";
	check(response == result, "build response line for 308");
	std::cout << "Result: " << result << std::endl;

	instructions.statusCode = 200;
	instructions.contentType = "text/plain";
	instructions.closeConnection = false;
	instructions.isRedirect = false;
	instructions.resolvedPath =
		"response/test_files/hello.txt";

	result = buildHttpResponse(instructions, "", "GET");

	response = "HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 5\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		"Hello";

	check(response == result, "build response with file body");
	std::cout << "Result: " << result << std::endl;

}

void	HttpResponseTests::testAutoIndex()
{
	t_responseInstructions instructions;

	instructions.statusCode = 200;
	instructions.isAutoIndex = true;
	instructions.resolvedPath = "response/test_files/fruits";
	instructions.contentType = "text/html";

	std::string result = buildHttpResponse(instructions, "", "GET");

	check(result.find("apple.html") != std::string::npos && result.find("banana.txt") != std::string::npos, "AutoIndex lists directory entries");
	std::cout << "Result: " << result << std::endl;
}

void	HttpResponseTests::testPost()
{
	t_responseInstructions instructions;
    instructions.statusCode = 201;
    instructions.contentType = "text/plain";
    instructions.resolvedPath = "response/test_files/post_test.txt";
    std::string requestBody = "Hello from POST!";
	std::string result = buildHttpResponse(instructions, requestBody, "POST");
	check(result.find("HTTP/1.1 201 Created\r\n") == 0,
		"POST returns 201 Created");

	std::ifstream file(instructions.resolvedPath.c_str());
	std::string savedBody;
	std::getline(file, savedBody);
	check(file.is_open() && savedBody == requestBody,
		"POST writes request body to file");
	check(result.find("\r\n\r\n") == result.size() - 4,
   		"POST response has an empty body");

	instructions.resolvedPath = "response/test_files/nonexistent_dir/post.txt";
	std::string failedResult =
		buildHttpResponse(instructions, requestBody, "POST");
	check(failedResult.find("HTTP/1.1 500 Internal Server Error\r\n") == 0,
		"POST returns 500 when file creation fails");
	std::remove("response/test_files/post_test.txt");
}

void	HttpResponseTests::testDelete()
{
	 const std::string path = "response/test_files/delete_test.txt";

    std::ofstream file(path.c_str());
    file << "Delete me";
    file.close();
    t_responseInstructions instructions;
    instructions.statusCode = 204;
    instructions.resolvedPath = path;
    instructions.contentType = "text/plain";
    std::string result = buildHttpResponse(instructions, "", "DELETE");
    check(result.find("HTTP/1.1 204 No Content\r\n") == 0,
        "DELETE returns 204 No Content");

    std::ifstream deletedFile(path.c_str());
    check(!deletedFile.is_open(),
        "DELETE removes the file");

	std::string failedResult = buildHttpResponse(instructions, "", "DELETE");
	check(failedResult.find("HTTP/1.1 500 Internal Server Error\r\n") == 0,
		"DELETE returns 500 when file removal fails");
}