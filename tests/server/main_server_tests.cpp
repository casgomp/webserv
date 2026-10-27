/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_server_tests.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 14:19:46 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/27 13:16:51 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerTests.hpp"

int	main()
{
	ServerTests st;

	st.run_all();

	if (st.getFailed() == 0)
		return (0);
	return(1);
}