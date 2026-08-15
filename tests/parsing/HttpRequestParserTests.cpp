/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParserTests.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 12:58:47 by pecastro          #+#    #+#             */
/*   Updated: 2026/08/15 13:13:32 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestParserTests.hpp"

HttpRequestParserTests::HttpRequestParserTests() : TestSuite("HttpRequestParserTests") {}

//add test methods here

void HttpRequestParserTests::run_all()
{
	std::cout << "Running HttpRequestParserTests..." << std::endl;
	//call each method here
	printSummary();
}
