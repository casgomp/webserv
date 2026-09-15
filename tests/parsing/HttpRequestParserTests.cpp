/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParserTests.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 12:58:47 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/15 10:55:21 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestParserTests.hpp"


HttpRequestParserTests::HttpRequestParserTests() : TestSuite("HttpRequestParserTests") {}

//add test methods here

void HttpRequestParserTests::run_all()
{
	std::cout << "Running HttpRequestParserTests..." << std::endl;

	//general
	testValidRequest();
	testIncompleteRequest();
	// Request line
	testMalformedMethod();
	testUnsupportedMethod();
	testOriginFormTarget();
	testHttpVersion();
	testRequestLineLength();
	//Headers and framing
	testHostHeader();
	testContentLength();
	testTransferEncoding();
	//Body
	testChunkedBody();
	//Request consumption/ pipelining
	testConsumedBytes();
	printSummary();
}

void	HttpRequestParserTests::testValidRequest()
{
	std::string request = "GET / HTTP/1.1\r\n"
							"Host: localhost\r\n\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0),
		"valid request returns COMPLETE with no error");
}

void	HttpRequestParserTests::testIncompleteRequest()
{
	std::string request = "GET / HTTP/1.1\r\n"
							"Host: localhost\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check((result == INCOMPLETE) && (httpRequest.statusCode == 0),
		"incomplete request returns INCOMPLETE with no error");

	request = "GET / HTTP/1.1";
	result = parseRequest(request, httpRequest);
	check((result == INCOMPLETE) && (httpRequest.statusCode == 0),
		"incomplete request line returns INCOMPLETE with no error");
}

void	HttpRequestParserTests::testMalformedMethod()
{
	std::string request = "G@T / HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"malformed method returns ERROR with 400");

	request = "GET  / HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"malformed request line spacing returns ERROR with 400");

	request = "GET / HTTP/1.1\n"
		"Host: localhost\r\n"
		"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"request line with bare LF returns ERROR with 400");
}

void	HttpRequestParserTests::testUnsupportedMethod()
{
	std::string request = "PUT / HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 501),
		"unsupported PUT returns ERROR with 501");
}

void	HttpRequestParserTests::testOriginFormTarget()
{
	std::string request = "GET /index.html HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0),
		"valid origin-form target returns COMPLETE with no error");

	request = "GET /hello%20world HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0),
		"valid percent-encoded target returns COMPLETE with no error");

	request = "GET /hello%GGworld HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"invalid percent-encoded target returns ERROR with 400");

	request = "GET /hello% HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"incomplete percent encoding returns ERROR with 400");

	request = "GET /hello world HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"target with space returns ERROR with 400");

	request = "GET /hello\tworld HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"target with tab returns ERROR with 400");


	request = "GET /search?q=hello HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0),
		"valid query target returns COMPLETE with no error");

	request = "GET /search?a=1?b=2 HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0),
		"query with additional '?' returns COMPLETE with no error");

	request = "GET /search?q={bad} HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"query with invalid characters returns ERROR with 400");

	request = "GET /hello#frag HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"target with fragment returns ERROR with 400");

	request = "GET /search?q=hello%20world HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0),
		"query with percent encoding returns COMPLETE with no error");
}

void	HttpRequestParserTests::testHttpVersion()
{
	std::string request = "GET /index.html HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0),
		"HTTP/1.1 returns COMPLETE with no error");

	request = "GET /index.html HTTP/1.0\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 505),
		"HTTP/1.0 returns ERROR with 505");
	request = "GET /index.html http/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"malformed HTTP version returns ERROR with 400");

	request = "GET / HTTP/2.0\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 505),
		"HTTP/2.0 returns ERROR with 505");
}

void	HttpRequestParserTests::testRequestLineLength()
{
	HttpRequest httpRequest;
	std::string line(8001, 'a');

	std::string request = "GET /" + line + " HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	ParseResult result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 414),
		"request line over 8000 bytes returns ERROR with 414");

	std::string line2(7986, 'a');

	request = "GET /" + line2 + " HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0),
		"request line exactly 8000 bytes returns COMPLETE with no error");

	std::string line3(7988, 'a');

	request = "GET /" + line3 + " HTTP/1.1";

	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 414),
		"overlong incomplete request line returns ERROR with 414");
}

void	HttpRequestParserTests::testHostHeader()
{
	std::string request = "GET /index.html HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0),
		"single valid Host header returns COMPLETE with no error");

	request = "GET /index.html HTTP/1.1\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"missing Host header returns ERROR with 400");

	request = "GET /index.html HTTP/1.1\r\n"
							"Host: \r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"empty Host header returns ERROR with 400");

	request = "GET /index.html HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"Host: example.com\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"duplicate Host header returns ERROR with 400");

	request = "GET /index.html HTTP/1.1\r\n"
			"HOST: localhost\r\n"
			"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0),
		"case-insensitive Host header returns COMPLETE with no error");

	request = "GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"InvalidHeader\r\n"
		"\r\n";
	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"header without colon returns ERROR with 400");
}

void	HttpRequestParserTests::testContentLength()
{
	std::string request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 5\r\n"
						"\r\n"
						"hello";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check((result == COMPLETE) &&
		(httpRequest.statusCode == 0) &&
		(httpRequest.body == "hello"),
		"Content-Length body returns COMPLETE with correct body");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 5\r\n"
						"\r\n"
						"hel";

	result = parseRequest(request, httpRequest);
	check((result == INCOMPLETE) && (httpRequest.statusCode == 0),
		"incomplete Content-Length body returns INCOMPLETE with no error");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: abc\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"invalid Content-Length returns ERROR with 400");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length:\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"empty Content-Length returns ERROR with 400");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 5\r\n"
						"Content-Length: 5\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0),
		"duplicate identical Content-Length returns COMPLETE with no error");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 5\r\n"
						"Content-Length: 6\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"conflicting Content-Length returns ERROR with 400");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 999999999999999999999999999999999999\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"overflowing Content-Length returns ERROR with 400");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 0\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0) &&
		(httpRequest.body.empty()),
		"zero Content-Length returns COMPLETE with empty body");
}

void	HttpRequestParserTests::testTransferEncoding()
{
	std::string request = "POST /upload HTTP/1.1\r\n"
				"Host: localhost\r\n"
				"Transfer-Encoding: chunked\r\n"
				"\r\n"
				"5\r\nhello\r\n"
				"6\r\n world\r\n"
				"0\r\n"
				"\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0) &&
		(httpRequest.body == "hello world"),
		"chunked request returns COMPLETE with correct body");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 5\r\n"
						"transfer-encoding: chunked\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"Content-Length with Transfer-Encoding returns ERROR with 400");


	request = "POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: gzip\r\n"
		"\r\n";

	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 501),
		"unsupported Transfer-Encoding returns ERROR with 501");

	request = "POST /upload HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Transfer-Encoding: chunked\r\n"
			"\r\n"
			"0\r\n"
			"\r\n";

	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"duplicate Transfer-Encoding returns ERROR with 400");
}

void HttpRequestParserTests::testChunkedBody()
{
	std::string request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"transfer-encoding: chunked\r\n"
						"\r\n"
						"5\r\n"
						"hello\r\n"
						"6\r\n"
						" world\r\n"
						"0\r\n"
						"\r\n";

	HttpRequest httpRequest;
	ParseResult result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0) &&
		(httpRequest.body == "hello world"),
		"chunked request with two chunks returns COMPLETE with correct body");

	request = "GET /index.html HTTP/1.1\r\n"
          "Host: localhost\r\n"
          "Transfer-Encoding: chunked\r\n"
          "\r\n"
          "5\r\n"
          "hello\r\n"
          "6\r\n"
          " wor";

	result = parseRequest(request, httpRequest);
	check((result == INCOMPLETE) && (httpRequest.statusCode == 0),
		"incomplete chunked body returns INCOMPLETE with no error");

	request = "GET /index.html HTTP/1.1\r\n"
          "Host: localhost\r\n"
          "Transfer-Encoding: chunked\r\n"
          "\r\n"
          "5\r\n"
          "helloXX"
          "0\r\n"
          "\r\n";

	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"malformed chunk terminator returns ERROR with 400");

	request = "GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"ZZ\r\n"
		"hello\r\n"
		"0\r\n"
		"\r\n";

	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"invalid chunk size returns ERROR with 400");

	request = "GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"5";

	result = parseRequest(request, httpRequest);
	check((result == INCOMPLETE) && (httpRequest.statusCode == 0),
		"incomplete chunk size line returns INCOMPLETE with no error");

	request = "GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"5\r\n"
		"hello\r\n"
		"0\r\n";

	result = parseRequest(request, httpRequest);
	check((result == INCOMPLETE) && (httpRequest.statusCode == 0),
		"incomplete final chunk returns INCOMPLETE with no error");

	request = "GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"A\r\n"
		"0123456789\r\n"
		"0\r\n"
		"\r\n";

	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0) &&
		(httpRequest.body == "0123456789"),
		"uppercase hex chunk size returns COMPLETE with correct body");

	request = "GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\r\n";

	result = parseRequest(request, httpRequest);
	check((result == ERROR) && (httpRequest.statusCode == 400),
		"overflowing chunk size returns ERROR with 400");
}

void	HttpRequestParserTests::testConsumedBytes()
{
	std::string request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"\r\n";

	HttpRequest httpRequest;
	ParseResult result = parseRequest(request, httpRequest);

	check((result == COMPLETE) && (httpRequest.statusCode == 0) &&
		(httpRequest.consumedBytes == request.size()),
		"complete request consumes exactly the request bytes");

	std::string firstRequest = "GET /first HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	std::string secondRequest = "GET /second HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	request = firstRequest + secondRequest;
	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0) &&
		(httpRequest.consumedBytes == firstRequest.size()),
		"pipelined request consumes only the first request");

	firstRequest = "POST /first HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Content-Length: 5\r\n"
			"\r\n"
			"hello";
	secondRequest = "GET /second HTTP/1.1\r\n"
				"Host: localhost\r\n"
				"\r\n";
	request = firstRequest + secondRequest;
	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0) &&
		(httpRequest.body == "hello") && (httpRequest.consumedBytes == firstRequest.size()),
		"Content-Length request consumes body but not pipelined request");

	firstRequest = "POST /first HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Transfer-Encoding: chunked\r\n"
			"\r\n"
			"5\r\n"
			"hello\r\n"
			"0\r\n"
			"\r\n";
	secondRequest = "GET /second HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"\r\n";
	request = firstRequest + secondRequest;
	result = parseRequest(request, httpRequest);
	check((result == COMPLETE) &&
		(httpRequest.statusCode == 0) && (httpRequest.body == "hello") &&
		(httpRequest.consumedBytes == firstRequest.size()),
		"chunked request consumes body but not pipelined request");

	std::string remaining = request.substr(httpRequest.consumedBytes);
	result = parseRequest(remaining, httpRequest);
	check((result == COMPLETE) && (httpRequest.statusCode == 0) &&
		(httpRequest.requestLine.target == "/second"),
		"second pipelined request parses from remaining bytes");
}
