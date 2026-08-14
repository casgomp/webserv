/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestSuite.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 15:57:55 by pecastro          #+#    #+#             */
/*   Updated: 2026/08/14 18:11:55 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TestSuite.hpp"

TestSuite::TestSuite(const std::string & suiteName) : 
	_suiteName(suiteName),
	_passed(0),
	_failed(0) {}

void	TestSuite::check(bool condition, const std::string & testName)
{
	if (condition)
	{
		std::cout << "  \033[32m[PASS]\033[0m  " << testName << std::endl;
		_passed ++;
	}
	else
	{
		std::cout << "  \033[31m[FAIL]\033[0m  " << testName << std::endl;
		_failed ++;
	}
}

int	TestSuite::getPassed() const
{
	return(_passed);
}

int	TestSuite::getFailed() const
{
	return(_failed);
}

void	TestSuite::printSummary() const
{
	std::cout << "\n" << _suiteName << ": "
		<< _passed << " passed, " << _failed << " failed\n" << std::endl;
}

TestSuite::~TestSuite() {}