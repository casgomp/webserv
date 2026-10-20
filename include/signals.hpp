/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 14:11:13 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/19 15:25:45 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNALS_HPP
# define SIGNALS_HPP

# include <csignal>

extern volatile sig_atomic_t	signalReceived;

void	signalHandler(int status);
void	setupSignals();

#endif