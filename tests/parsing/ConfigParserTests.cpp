/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserTests.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 11:29:09 by pecastro          #+#    #+#             */
/*   Updated: 2026/08/17 14:43:05 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParserTests.hpp"

ConfigParserTests::ConfigParserTests() : TestSuite("ConfigParserTests") {}

//add test methods here

//basic stucture

//port

void ConfigParserTests::run_all()
{
	std::cout << "Running ConfigParserTests..." << std::endl;
	//call each method here
	printSummary();
}