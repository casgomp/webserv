/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserTests.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 11:28:56 by pecastro          #+#    #+#             */
/*   Updated: 2026/08/15 11:46:53 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_PARSER_TESTS_HPP
# define CONFIG_PARSER_TESTS_HPP

#include "../TestSuite.hpp"

class ConfigParserTests : public TestSuite {
	public:
		ConfigParserTests();

		//add test methods here

		void run_all();
};


#endif