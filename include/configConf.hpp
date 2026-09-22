/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 13:29:25 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/22 14:57:05 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_CONF_HPP
# define CONFIG_CONF_HPP

# include <string>
# include <vector>

typedef struct	s_locationConf {
	std::string							root; //inherit
	size_t								clientMaxBodySize; //inherit
	bool								autoindex; //inherit
	std::vector<std::string>			index; //inherit
	std::string							path;
	std::vector<std::string>			allowedMethods;
	std::pair<int, std::string>			redirection;
	bool								isCgi;
	s_locationConf() : clientMaxBodySize(0), autoindex(false), isCgi(false)
	{
		allowedMethods.push_back("GET");
		allowedMethods.push_back("POST");
		allowedMethods.push_back("DELETE");
	}
} t_locationConf;

typedef struct	s_serverConf {
	std::string											root; //inherit
	size_t												clientMaxBodySize; //inherit
	bool												autoindex; //inherit
	std::vector<std::string>							index; //inherit
	std::vector<std::string>							serverNames;
	std::vector<std::pair<std::string, std::string> >	listen;
	std::vector<t_locationConf>							locations;
	s_serverConf() : clientMaxBodySize(0), autoindex(false) {}
} t_serverConf;

typedef struct	s_httpConf {
	std::string							root; //inherit
	size_t								clientMaxBodySize; //inherit
	// Sets the maximum allowed size of the client request body.
	// If the size in a request exceeds the configured value, the 413.
	// (Request Entity Too Large) error is returned to the client.
	// Please be aware that browsers cannot correctly display this error.
	// Setting size to 0 disables checking of client request body size.
	bool								autoindex; //inherit
	std::vector<std::string>			index;
	std::vector<t_serverConf>			servers;
	s_httpConf() : clientMaxBodySize(0), autoindex(false) {}
} t_httpConf;

#endif
