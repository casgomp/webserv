/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_server_tests.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 14:19:46 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/04 17:12:44 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerInitTests.hpp"

int	main()
{
	ServerInitTests sit;

	sit.run_all();

	if (sit.getFailed() + sit.getFailed() == 0)
		return (0);
	return(1);
}