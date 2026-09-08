/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInitTests.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 11:28:56 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/05 18:26:54 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_INIT_TESTS_HPP
# define SERVER_INIT_TESTS_HPP

#include "../TestSuite.hpp"

class ServerInitTests : public TestSuite {
	public:
		ServerInitTests();

		//add test methods here
		void	test_getListenServers();
		void	test_serverInit();

		void	run_all();
};

#endif