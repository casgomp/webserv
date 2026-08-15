/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_parsing_tests.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 14:19:46 by pecastro          #+#    #+#             */
/*   Updated: 2026/08/15 14:38:26 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParserTests.hpp"
#include "HttpRequestParserTests.hpp"

int	main()
{
	ConfigParserTests cpt;
	HttpRequestParserTests hrpt;

	cpt.run_all();
	hrpt.run_all();

	if (cpt.getFailed() + hrpt.getFailed() == 0)
		return (0);
	return(1);
}