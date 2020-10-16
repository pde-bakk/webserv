/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parser.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: peerdb <peerdb@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/09/29 16:36:33 by peerdb        #+#    #+#                 */
/*   Updated: 2020/10/13 17:06:21 by pde-bakk      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <sys/stat.h>
#include <Server.hpp>
#include "libftGnl.hpp"

int		is_first_char(std::string str, char find) {
	int i = 0;
	while (str[i] && iswhitespace(str[i]))
		++i;
	if (str[i] == find)
		return (1);
	return (0);
}

void	get_key_value(std::string &str, std::string &key, std::string& value) {
	size_t kbegin = str.find_first_not_of(" \t\n");
	size_t kend = str.find_first_of(" \t\n", kbegin);
	key = str.substr(kbegin, kend - kbegin);
	size_t vbegin = str.find_first_not_of(" \t\n", kend);
	size_t vend = str.find_first_of("\n\r#;", vbegin);
	value = str.substr(vbegin, vend - vbegin);
}

Servermanager	parse(char *av) {
	Servermanager	skrtks;
	std::string		str;
	struct stat statstruct = {};
	int fd;
	if (av && stat(av, &statstruct) != -1)
		fd = open(av, O_RDONLY);
	else
		fd = open("configfiles/nginx.conf", O_RDONLY);
	if (fd < 0)
		return skrtks;
	
	while (ft::get_next_line(fd, str) > 0) {
		if (is_first_char(str) || str.empty())
			continue ;
		try {
			Server	tmp;
			if (!str.empty() && str.compare(str.find_first_not_of(" \t\n"), ft_strlen("server {"), "server {") == 0) {
				tmp.setup(fd);
				skrtks += tmp;
			}
		}
		catch (std::exception& e) {
			std::cerr << "threw an exception in parse try block: " << e.what() << std::endl;
			close(fd);
			exit(1);
		}
	}
	close(fd);
	return skrtks;
}
