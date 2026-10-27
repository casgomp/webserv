/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestValidationUtils.hpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 11:16:23 by pecastro          #+#    #+#             */
/*   Updated: 2026/10/27 13:53:37 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_VALIDATION_UTILS_HPP
# define REQUEST_VALIDATION_UTILS_HPP

# include <cstring>
# include <map>
# include <string>

# include "configConf.hpp"

t_locationConf*						matchLocation(std::vector<t_locationConf> &locations, const std::string &target);
int									validateMethod(const std::vector<std::string> &allowedMethods, const std::string &requestMethod);
std::string							createRedirectPath(t_locationConf *location);
void								normalizePath(const std::string &target, std::string &normalizedTarget);
std::string							joinedPath(std::string root, std::string normalizedTarget);
bool								pathIsFile(const std::string &resolvedPath);
bool								pathIsDir(const std::string &resolvedPath);
std::map<std::string, std::string>	getValidMimeTypes();
std::string							getContentType(const std::string &path);

#endif
