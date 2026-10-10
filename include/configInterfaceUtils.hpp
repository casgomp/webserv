/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configInterfaceUtils.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:13:18 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/03 14:14:06 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_INTERFACE_UTILS_HPP
# define CONFIG_INTERFACE_UTILS_HPP

# include <sstream>
# include <string>
# include <vector>

# include "configConf.hpp"

void						checkIfValidDir(const std::string &path);
int							strToNum(const std::string &str);
int							checkAutoindex(const std::string &autoindex);
std::vector<std::string>	checkIndexFiles(const std::string &input);
void						addServerNames(t_serverConf &serverConf, const std::string &input);
void						addListenAddressPort(t_serverConf &serverConf, const std::string &input);
void						addAllowedMethods(t_locationConf &locationConf, const std::string &input);
void						addRedirection(t_locationConf &locationConf, const std::string &input);
void						addCgiExtension(t_locationConf &locationConf, const std::string &input);

#endif
