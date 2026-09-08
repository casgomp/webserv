/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParserTests.hpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 12:58:58 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/02 10:45:55 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_PARSER_TESTS_HPP
# define HTTP_REQUEST_PARSER_TESTS_HPP

#include "../TestSuite.hpp"
#include "../../include/httpRequestParser.hpp"

class HttpRequestParserTests : public TestSuite {
	public:
		HttpRequestParserTests();

		//add test methods here

		void testValidGet(); 
		void testIncompleteRequest();
		void testMalformedMethod();
		void testPutMethod();
		void testOriginFormTarget();
		void testVersionRequestLine();
		void testHostHeader();
		void testContentLength();
		void run_all();
};


#endif
