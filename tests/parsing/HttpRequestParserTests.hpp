/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParserTests.hpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 12:58:58 by pecastro          #+#    #+#             */
/*   Updated: 2026/08/15 13:13:30 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_PARSER_TESTS_HPP
# define HTTP_REQUEST_PARSER_TESTS_HPP

#include "../TestSuite.hpp"

class HttpRequestParserTests : public TestSuite {
	public:
		HttpRequestParserTests();

		//add test methods here

		void run_all();
};


#endif
