/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 14:10:56 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/19 15:31:13 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <csignal>

# include "signals.hpp"

volatile sig_atomic_t	signalReceived = 0;

void	signalHandler(int status)
{
    signalReceived = status;
}

void	setupSignals()
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGPIPE, SIG_IGN);
}