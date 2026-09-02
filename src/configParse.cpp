/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configParse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pecastro <pecastro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 15:32:38 by pecastro          #+#    #+#             */
/*   Updated: 2026/09/02 18:04:33 by pecastro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

void	printConfig(t_block conf, int depth)
{
	std::string	indent(depth * 2, ' ');
	size_t		i;

	i = 0;
	while (i < conf.directives.size())
	{
		std::cout << indent << conf.directives[i].first << " = " << conf.directives[i].second << std::endl;
		i ++;
	}
	if (!(conf.children.empty()))
		std::cout << indent << "**children" << std::endl;
	i = 0;
	while (i < conf.children.size())
	{
		std::cout << indent << "  " << conf.children[i].first.first << " = " << conf.children[i].first.second << std::endl;
		if (!(conf.children[i].second.directives.empty() && conf.children[i].second.children.empty()))
			printConfig(conf.children[i].second, depth + 1);
		i ++;
	}
}

std::pair<std::string, std:: string>	splitter(const std::string &chunk)
{
	size_t		start;
	size_t		found;
	size_t		end;
	std::string	chunkTrimmed;
	std::string	keyword;
	std::string	params;

	// std::cout << "chunk = " << chunk << std::endl;
	start = chunk.find_first_not_of(" \t\n", 0);
	if (start == std::string::npos)
			return (std::make_pair("", ""));
	end = chunk.find_last_not_of(" \t\n", std::string::npos) + 1;
	// std::cout << "start = " << start << " ...end = " << end << std::endl;
	chunkTrimmed = chunk.substr(start, end - start);
	found = chunkTrimmed.find_first_of(" \t\n", 0);
	if (found != std::string::npos)
	{
		keyword = chunkTrimmed.substr(0, found);
		start = chunkTrimmed.find_first_not_of(" \t\n", found);
		params = chunkTrimmed.substr(start, end - start);
	}
	else
		keyword = chunkTrimmed.substr(0, end);
	return (std::make_pair(keyword, params));
}

t_block	recurseConfig(const std::string &str)
{
	std::string							terminator;
	size_t								found;
	size_t								start;
	std::string							chunk;
	std::pair<std::string, std::string>	pairDirective;
	t_block								parsedData;
	int									depthCounter;
	size_t								foundDepthCounter;
	size_t								startDepthCounter;
	size_t								startBlockDepthCounter;
	size_t								terminatorDepthCounter;

	start = 0;
	while (1)
	{
		found = str.find_first_of(";{", start);
		if  (found == std::string::npos)
			break ;
		terminator = str.at(found);
		chunk = str.substr(start, found - start);

		pairDirective = splitter(chunk);
		std::cout << "pairDirective.first = " << pairDirective.first << std::endl;
		if (pairDirective.first.empty())
			throw std::runtime_error("empty block directive name in config");

		if (terminator == ";")
		{
			parsedData.directives.push_back(pairDirective);
			start = found + 1;
		}
		else if (terminator == "{")
		{
			depthCounter = 0;
			startDepthCounter = found;
			startBlockDepthCounter = found + 1;
			while (1)
			{
				foundDepthCounter = str.find_first_of("{}", startDepthCounter);
				if (foundDepthCounter == std::string::npos)
					break ;
				else if (str.at(foundDepthCounter) == '{')
					depthCounter ++;
				else if (str.at(foundDepthCounter) == '}')
				{
					terminatorDepthCounter = foundDepthCounter;
					depthCounter --;
					if (depthCounter == 0)
						break ;
				}
				startDepthCounter = foundDepthCounter + 1;
			}
			if (depthCounter == 0)
			{
				//why not just do blockNameParams = pairDirective? or just use pairDirective straight away?
				std::pair<std::string, std::string> blockNameParams = std::make_pair(pairDirective.first, pairDirective.second);
				t_block blockChild = recurseConfig(str.substr(startBlockDepthCounter, terminatorDepthCounter - startBlockDepthCounter));
				std::pair<std::pair<std::string, std::string>, t_block> pairChild = std::make_pair(blockNameParams, blockChild);
				parsedData.children.push_back(pairChild);
				start = terminatorDepthCounter + 1;
			}
			else
				throw std::runtime_error("unmatched '{' in config");
		}
	}
	return (parsedData);
}

int	readConfigToString(const char *filename, std::string &str)
{
	std::ifstream	ifs;
	std::string		tmp;

	ifs.open(filename);
	if (!ifs.is_open())
		return (1);
	while(getline(ifs, tmp))
		str += tmp;
	// std::cout << str << std::endl;//debugging
	return(0);
}

t_block	parseConfig(const char *filename)
{
	t_block		conf;
	std::string	str;

	if (readConfigToString(filename, str) != 0)
	{
		std::cerr << "processConfigFile: failed to open/read config file: " << filename << std::endl;
		return (conf);//check if return correct and if empty?
	}
	conf = recurseConfig(str);

	//call function to create config-server interface
	//somestructure confInt = confInterface(conf);

	//maybe here continue with next step of parsing, i.e., call function that will do semantic analysis

	return (conf);//return confInterface
}