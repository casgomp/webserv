/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserTests.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 11:28:56 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/03 18:04:41 by pecastro         ###   ########.fr       */
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
		void	test_strToNum();
		void	test_checkAutoindex();
		void	test_addServerNames();
		void	test_addListenAddressPort();
		void	test_addErrorPages();
		void	test_addAllowedMethods();
		void	test_addRedirection();

		void	run_all();
};

#endif