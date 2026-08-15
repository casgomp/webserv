/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestSuite.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 15:58:04 by pecastro          #+#    #+#             */
/*   Updated: 2026/08/14 18:16:39 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEST_SUITE_HPP
# define TEST_SUITE_HPP

#include <iostream>
#include <string>

class TestSuite {
	private:
		std::string	_suiteName;
		int			_passed;
		int			_failed;
	protected:
		void		check(bool condition, const std::string & testName);
	public:
		TestSuite(const std::string & suiteName);
		virtual		~TestSuite();

		int			getPassed() const;
		int			getFailed() const;
		void		printSummary() const;
};

#endif
