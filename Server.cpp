/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: pde-bakk <pde-bakk@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/10/07 12:57:25 by pde-bakk      #+#    #+#                 */
/*   Updated: 2020/10/07 19:13:41 by pde-bakk      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(void) : _port(80), _host("127.0.0.1"),
		_client_body_size(1000000), _error_page("error.html") {
	// default nginx values
	// client_body_size = 1Megabyte
}

Server::~Server(void) {

}

Server::Server(const Server& x) {
	*this = x;
}

Server&	Server::operator=(const Server& x) {
	if (this != &x) {
		this->_port = x._port;
		this->_host = x._host;
		this->_server_name = x._server_name;
		this->_client_body_size = x._client_body_size;
		this->_error_page = x._error_page;
	}
	return *this;
}

size_t	Server::getport() const {
	return this->_port;
}

void	Server::setport(const std::string& port) {
	this->_port = ft_atoi(port.c_str());
}

std::string	Server::gethost() const {
	return this->_host;
}

void	Server::sethost(const std::string& host) {
	this->_host = host;
}

std::string	Server::getservername() const {
	return this->_server_name;
}

void	Server::setservername(const std::string& servername) {
	this->_server_name = servername;
	std::cout << "server name is " << _server_name << std::endl;
}

size_t	Server::getclientbodysize() const {
	return this->_client_body_size;
}

void	Server::setclientbodysize(const std::string& clientbodysize) {
	this->_client_body_size = ft_atoi(clientbodysize.c_str());
	size_t i = clientbodysize.find_first_not_of("1234567890");
	if (clientbodysize[i] == 'M')
		this->_client_body_size *= 1000000;
}

std::string	Server::geterrorpage() const {
	return this->_error_page;
}

void	Server::seterrorpage(const std::string& errorpage) {
	this->_error_page = errorpage;
}

void	Server::setup(int fd) {
	std::map<std::string, void (Server::*)(const std::string&)> m;
	m["port"] = &Server::setport;
	m["host"] = &Server::sethost;
	m["server_name"] = &Server::setservername;
	m["LIMIT_CLIENT_BODY_SIZE"] = &Server::setclientbodysize;
	m["DEFAULT_ERROR_PAGE"] = &Server::seterrorpage;
	std::string str;

	while (get_next_line(fd, str) > 0) {
		std::string key, value;
		if (is_first_char(str)) //checks for comment char #
			continue ;
		if (is_first_char(str, '}'))
			break ;
		try {
			get_key_value(str, key, value);
			// std::cout << "key = " << key << ", value = " << value << "$" << std::endl;
			(this->*(m.at(key)))(value); // (this->*(m[key]))(value);
		}
		catch (std::exception& e) {
			std::cerr << "exception: " << e.what() << std::endl << std::endl;
		}
	}
	std::cout << this->getservername() <<  " is listening on: " << this->gethost() << ":" << this->getport() << std::endl;
	std::cout << "error page = " << this->geterrorpage() << std::endl;
	std::cout << "client body limit = " << this->getclientbodysize() << std::endl;
}
