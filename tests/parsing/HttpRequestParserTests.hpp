/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParserTests.hpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 12:58:58 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/15 10:44:55 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_PARSER_TESTS_HPP
# define HTTP_REQUEST_PARSER_TESTS_HPP

#include "../TestSuite.hpp"
#include "../../include/httpRequestParser.hpp"

class HttpRequestParserTests : public TestSuite {
	public:
		HttpRequestParserTests();

		// General
		void testValidRequest();
		void testIncompleteRequest();

		// Request Line
		void testMalformedMethod();
		void testUnsupportedMethod();
		void testOriginFormTarget();
		void testHttpVersion();
		void testRequestLineLength();

		// Headers and framing
		void testHostHeader();
		void testContentLength();
		void testTransferEncoding();

		// Body
		void testChunkedBody();

		// Request consumption / pipelining
		void testConsumedBytes();

		void run_all();
};


#endif
