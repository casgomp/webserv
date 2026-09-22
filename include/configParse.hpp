/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configParse.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:13:03 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/22 11:30:06 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_PARSE_HPP
# define CONFIG_PARSE_HPP

# include <string>
# include <vector>

typedef struct	s_block {
	std::vector<std::pair<std::string, std::string> >						directives;
	std::vector<std::pair<std::pair<std::string, std::string>, s_block> >	children;
} t_block;

t_block									parseConfig(const char *filename);
void									readConfigToString(const char *filename, std::string &str);
t_block									recurseConfig(const std::string &str);
std::pair<std::string, std:: string>	splitter(const std::string &chunk);
void									printConfig(t_block conf, int depth = 0);

#endif