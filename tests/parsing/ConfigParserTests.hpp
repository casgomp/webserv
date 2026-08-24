/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserTests.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 11:28:56 by pecastro          #+#    #+#             */
/*   Updated: 2026/08/24 16:59:37 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_PARSER_TESTS_HPP
# define CONFIG_PARSER_TESTS_HPP

#include "../TestSuite.hpp"

class ConfigParserTests : public TestSuite {
	public:
		ConfigParserTests();

		//add test methods here

		void	test_split_chunk();
		void	test_read_valid_file_correctly();

		void	run_all();
};

#endif