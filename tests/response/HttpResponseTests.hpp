/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseTests.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 11:36:41 by erjonbara         #+#    #+#             */
/*   Updated: 2026/10/02 18:23:12 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_RESPONSE_TESTS_HPP
# define HTTP_RESPONSE_TESTS_HPP

# include "../TestSuite.hpp"


class HttpResponseTests : public TestSuite {
	public:
		HttpResponseTests();
		void	validateResponseLine();
		void	testAutoIndex();
		void	testGet();
		void	testPost();
		void	testDelete();
		void	testErrorResponses();
		void	run_all();
};

#endif