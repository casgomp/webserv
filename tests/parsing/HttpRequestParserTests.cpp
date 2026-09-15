/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParserTests.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 12:58:47 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/13 17:42:22 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestParserTests.hpp"


HttpRequestParserTests::HttpRequestParserTests() : TestSuite("HttpRequestParserTests") {}

//add test methods here

void HttpRequestParserTests::run_all()
{
	std::cout << "\n\033[30;105mRunning HttpRequestParserTests...\033[0m\n" << std::endl;

	//call each method here

	testValidGet();
	testIncompleteRequest();
	testMalformedMethod();
	testPutMethod();
	testOriginFormTarget();
	testVersionRequestLine();
	testHostHeader();
	testContentLength();
	printSummary();
}

void	HttpRequestParserTests::testValidGet()
{
	std::string request = "GET / HTTP/1.1\r\n"
							"Host: localhost\r\n\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check(result == PARSE_COMPLETE, "test valid get");
}

void	HttpRequestParserTests::testIncompleteRequest()
{
	std::string request = "GET / HTTP/1.1\r\n"
							"Host: localhost\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check(result == PARSE_INCOMPLETE, "test Incomplete Get");
}

void	HttpRequestParserTests::testMalformedMethod()
{
	std::string request = "G@T / HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test malformed method");
}

void	HttpRequestParserTests::testPutMethod()
{
	std::string request = "PUT / HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test PUT method");
}

void	HttpRequestParserTests::testOriginFormTarget()
{
	std::string request = "GET /index.html HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check(result == PARSE_COMPLETE, "test simple origin form");

	request = "GET /hello%20world HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_COMPLETE, "test simple with hexDigit origin form");

	request = "GET /hello%GGworld HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test bad origin form");

	request = "GET /hello% HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test bad origin form");

	request = "GET /hello world HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test bad origin form");

	request = "GET /hello\tworld HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test bad origin form with tab");

	// TEst invalid query

	request = "GET /search?q=hello HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_COMPLETE, "test valid query");

	request = "GET /search?a=1?b=2 HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_COMPLETE, "test additional ? inside query is allowed");

	request = "GET /search?q={bad} HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test bad query syntax");

	request = "GET /hello#frag HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test bad query syntax");

	request = "GET /search?q=hello%20world HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_COMPLETE, "test query syntax with '%'");
}

void	HttpRequestParserTests::testVersionRequestLine()
{
	std::string request = "GET /index.html HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check(result == PARSE_COMPLETE, "test HTTP/1.1");

	request = "GET /index.html HTTP/1.0\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test HTTP/1.0");

	request = "GET /index.html http/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test http/1.1");

	request = "GET / HTTP/2.0\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test HTTP/2.0");

	// Test request line size
	std::string line(8001, 'a');

	request = "GET /" + line + " HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "request line more than 8000 bytes");

	std::string line2(7986, 'a');

	request = "GET /" + line2 + " HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_COMPLETE, "request exactly 8000 bytes");
}

void	HttpRequestParserTests::testHostHeader()
{
	std::string request = "GET /index.html HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"\r\n";
	HttpRequest httpRequest;

	ParseResult result = parseRequest(request, httpRequest);
	check(result == PARSE_COMPLETE, "test with 1 valid host header");

	request = "GET /index.html HTTP/1.1\r\n"
							// "Host: localhost\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test without host header");

	request = "GET /index.html HTTP/1.1\r\n"
							"Host: \r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test with empty host header");

	request = "GET /index.html HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"Host: example.com\r\n"
							"\r\n";
	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test with duplicate host header");
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
	check(httpRequest.body == "hello", "content length body value");
	check(result == PARSE_COMPLETE, "test with body full bytes");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 5\r\n"
						"\r\n"
						"hel";

	result = parseRequest(request, httpRequest);
	check(result == PARSE_INCOMPLETE, "test with body less bytes");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: abc\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test with invalid length");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length:\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test with empty length");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 5\r\n"
						"Content-Length: 5\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check(result == PARSE_COMPLETE, "test with duplicate content length same value");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 5\r\n"
						"Content-Length: 6\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test with different length");

	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 999999999999999999999999999999999999\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test with very large length");


	// CL && TE checks

	request = "POST /upload HTTP/1.1\r\n"
				"Host: localhost\r\n"
				"Transfer-Encoding: chunked\r\n"
				"\r\n"
				"5\r\nhello\r\n"
				"6\r\n world\r\n"
				"0\r\n"
				"\r\n";

	result = parseRequest(request, httpRequest);
	check(result == PARSE_COMPLETE, "test TE: chunked");


	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"Content-Length: 5\r\n"
						"transfer-encoding: chunked\r\n"
						"\r\n"
						"hello\r\n";

	result = parseRequest(request, httpRequest);
	check(result == PARSE_BAD_REQUEST, "test with TE && CL headers");


	//check one request with two chunks
	request = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost\r\n"
						"transfer-encoding: chunked\r\n"
						"\r\n"
						"5\r\n"
						"hello\r\n"
						"6\r\n"
						" world\r\n"
						"0\r\n"
						"\r\n";

	result = parseRequest(request, httpRequest);

	check(result == PARSE_COMPLETE,
		"chunked request with two chunks");

	check(httpRequest.body == "hello world",
		"chunked body contains both chunks");

	request = "GET /index.html HTTP/1.1\r\n"
          "Host: localhost\r\n"
          "Transfer-Encoding: chunked\r\n"
          "\r\n"
          "5\r\n"
          "hello\r\n"
          "6\r\n"
          " wor";

	result = parseRequest(request, httpRequest);

	check(result == PARSE_INCOMPLETE,
		"incomplete chunked body");

	request = "GET /index.html HTTP/1.1\r\n"
          "Host: localhost\r\n"
          "Transfer-Encoding: chunked\r\n"
          "\r\n"
          "5\r\n"
          "helloXX"
          "0\r\n"
          "\r\n";

	result = parseRequest(request, httpRequest);

	check(result == PARSE_BAD_REQUEST,
		"malformed chunk terminator");
}