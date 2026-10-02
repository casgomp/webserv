/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_response_tests.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erjonbara <erjonbara@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 11:36:55 by erjonbara         #+#    #+#             */
/*   Updated: 2026/09/21 12:36:00 by erjonbara        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponseTests.hpp"

int	main()
{
	HttpResponseTests hrt;

	hrt.run_all();

	if (hrt.getFailed() == 0)
		return (0);
	return(1);
}