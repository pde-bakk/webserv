/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ResponseHandler.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: skorteka <skorteka@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/10/08 16:15:11 by skorteka      #+#    #+#                 */
/*   Updated: 2020/10/29 11:08:24 by tuperera      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseHandler.hpp"
#include "libftGnl.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "Base64.hpp"
#include "Colours.hpp"

std::string getCurrentDatetime() {
	time_t		time;
	char 		datetime[100];
	tm*			curr_time;

	std::time(&time);
	curr_time = std::localtime(&time);
	std::strftime(datetime, 100, "%a, %d %B %Y %H:%M:%S GMT", curr_time);
	return datetime;
}

ResponseHandler::ResponseHandler() : _cgi_status_code() {
	this->_status_code = 200;
	_header_vals[ACCEPT_CHARSET].clear();
	_header_vals[ACCEPT_LANGUAGE].clear();
	_header_vals[ALLOW].clear();
	_header_vals[AUTHORIZATION].clear();
	_header_vals[CONNECTION] = "close";
	_header_vals[CONTENT_LANGUAGE].clear();
	_header_vals[CONTENT_LENGTH].clear();
	_header_vals[CONTENT_LOCATION].clear();
	_header_vals[CONTENT_TYPE].clear();
	_header_vals[DATE].clear();
	_header_vals[HOST].clear();
	_header_vals[LAST_MODIFIED].clear();
	_header_vals[LOCATION].clear();
	_header_vals[RETRY_AFTER].clear();
	_header_vals[SERVER] = "Webserv/1.0";
	_header_vals[TRANSFER_ENCODING].clear();
	_header_vals[WWW_AUTHENTICATE].clear();

	_status_codes[100] = "100 Continue\r\n"; // Only a part of the request has been received by the server, but as long as it has not been rejected, the client should continue with the request.
	_status_codes[200] = "200 OK\r\n";
	_status_codes[201] = "201 Created\r\n";
	_status_codes[202] = "202 Accepted\r\n"; // The request is accepted for processing, but the processing is not complete.
	_status_codes[203] = "203 Non-Authoritative Information\r\n"; // The information in the entity header is from a local or third-party copy, not from the original server.
	_status_codes[204] = "204 No Content\r\n";
	_status_codes[300] = "300 Multiple Choices\r\n"; // A link list. The user can select a link and go to that location. Maximum five addresses  .
	_status_codes[301] = "301 Moved Permanently\r\n";
	_status_codes[302] = "302 Found\r\n"; // The requested page has moved temporarily to a new url .
	_status_codes[307] = "307 Temporary Redirect\r\n"; // The requested page has moved temporarily to a new url.
	_status_codes[400] = "400 Bad Request\r\n"; // The server did not understand the request.
	_status_codes[404] = "404 Not Found\r\n";
	_status_codes[405] = "405 Method Not Allowed\r\n";
	_status_codes[406] = "406 Not Acceptable\r\n";
	_status_codes[407] = "407 Proxy Authentication Required\r\n";
	_status_codes[408] = "408 Request Timeout\r\n";
	_status_codes[409] = "409 Conflict\r\n"; // The request could not be completed because of a conflict.
	_status_codes[410] = "410 Gone\r\n"; // The requested page is no longer available .
	_status_codes[411] = "411 Length Required\r\n"; // The "Content-Length" is not defined. The server will not accept the request without it .
	_status_codes[413] = "413 Request Entity Too Large\r\n";
	_status_codes[414] = "414 Request-url Too Long\r\n";
	_status_codes[415] = "415 Unsupported Media Type\r\n";
	_status_codes[500] = "500 Internal Server Error\r\n"; // The request was not completed. The server met an unexpected condition.
	_status_codes[501] = "501 Not Implemented\r\n"; // The request was not completed. The server did not support the functionality required.
	_status_codes[502] = "502 Bad Gateway\r\n"; // The request was not completed. The server received an invalid response from the upstream server.
	_status_codes[503] = "503 Service Unavailable\r\n"; // The request was not completed. The server is temporarily overloading or down.
	_status_codes[504] = "504 Gateway Timeout\r\n";
	_status_codes[505] = "505 HTTP Version Not Supported\r\n";
}

ResponseHandler::~ResponseHandler() {
	this->_header_vals.clear();
	this->_cgi_headers.clear();
	this->_status_codes.clear();
	this->_response.clear();
	this->_body.clear();
}

ResponseHandler::ResponseHandler(const ResponseHandler &src) : _cgi_status_code(), _status_code() {
	*this = src;
}

ResponseHandler& ResponseHandler::operator= (const ResponseHandler &rhs) {
	if (this != &rhs) {
		this->_header_vals = rhs._header_vals;
		this->_cgi_headers = rhs._cgi_headers;
		this->_cgi_status_code = rhs._cgi_status_code;
		this->_status_codes = rhs._status_codes;
		this->_response	= rhs._response;
		this->_body = rhs._body;
		this->_status_code = rhs._status_code;
		this->CGI = rhs.CGI;
	}
	return *this;
}

int ResponseHandler::generatePage(request_s& request) {
	int			fd = -1;
	struct stat statstruct = {};
	std::cout << this->_autoindex << std::endl;

	if (request.server.isExtensionAllowed(request.uri)) {
		
		std::string scriptpath = request.uri.substr(1, request.uri.find_first_of('/', request.uri.find_first_of('.') ) - 1);
		if (request.uri.compare(0, 9, "/cgi-bin/") == 0 && request.uri.length() > 9) { // Run CGI script that creates an html page
			fd = this->CGI.run_cgi(request, scriptpath, request.uri);
		}
		else {
			std::string tmpuri = '/' + request.server.matchlocation(request.uri).getroot();
			size_t second_slash_index = request.uri.find_first_of('/', 1);
			if (second_slash_index == std::string::npos)
				tmpuri += request.uri;
			else
				tmpuri += request.uri.substr(second_slash_index);
			scriptpath = tmpuri.substr(1, tmpuri.find_first_of('/', tmpuri.find_first_of('.') ) - 1);
			if (stat(scriptpath.c_str(), &statstruct) == -1) {
				std::string defaultcgipath = request.server.matchlocation(request.uri).getdefaultcgipath();
				if (defaultcgipath.empty()) {
					fd = -2;
				}
				else {
					second_slash_index = tmpuri.find_first_of('/', 1);
					tmpuri.replace(second_slash_index + 1, tmpuri.find_first_of('/', second_slash_index), defaultcgipath);
				}
			}
			if (fd != -2) {
				std::string	OriginalUri(request.uri);
				request.uri = tmpuri;
				scriptpath = request.uri.substr(1, request.uri.find_first_of('/', request.uri.find_first_of('.')) - 1);
				fd = this->CGI.run_cgi(request, scriptpath, OriginalUri);
			}
		}
	}
	else {
		if (this->_autoindex)
			fd = request.server.getpage(request.uri, _header_vals, _status_code, true);
		else
			fd = request.server.getpage(request.uri, _header_vals, _status_code, false);
	}
	if (fd == -1)
		throw std::runtime_error(strerror(errno)); // cant even serve the error page, so I throw an error
	if (fd == -2) {
		fd = open(request.server.geterrorpage().c_str(), O_RDONLY);
		_status_code = 404;
	}
	return (fd);
}

void ResponseHandler::extractCGIheaders(const std::string& incoming) {
	RequestParser TMP;
	std::vector<std::string> vec = ft::split(incoming, "\n");
	for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); ++it) {
		if ((*it).find(':') == std::string::npos)
			continue;
		std::string key, value;
		get_key_value(*it, key, value, ":");
		ft::stringtoupper(key);
		ft::trimstring(value);
		std::map<std::string, headerType>::iterator header = TMP._headerMap.find(key);
		if (header != TMP._headerMap.end()) {
			_cgi_headers[header->second] = value;
		}
		else if (key == "STATUS")
			_cgi_status_code = ft_atoi(value.c_str());
	}
}

int findNthOccur(std::string str, char ch, int N)
{
    int occur = 0; 
  
    // Loop to find the Nth 
    // occurence of the character 
    for (int i = (str.length()); i >= 0; i--) { 
        if (str[i] == ch) { 
            occur += 1; 
        } 
		std::cout << occur << " " << i << std::endl;
        if (occur == N) 
            return i; 
    } 
    return -1; 
}

void ResponseHandler::handleAutoIndex(request_s& request) {
	DIR							*dir;
	char						cwd[2048];
	struct dirent				*entry;
	struct stat 				stats;
	struct tm					dt;
	std::stringstream 			ss;
	std::string 				path;
	std::string					months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	std::string 				url = "http://";
	std::string					s;

	if (request.uri[request.uri.length() - 1] != '/')
		request.uri += "/";
	s = request.uri;
	if (getcwd(cwd, sizeof(cwd)) == NULL)
    	perror("getcwd() error");
	else {
		path += cwd;
		path += "/";
	}
	path += request.server.getroot() + request.uri;
	dir = opendir(path.c_str());

	ss << request.server.getport();
	url += request.server.gethost() + ":" + ss.str();
	s = s.substr(0, findNthOccur(s, '/', 2) + 1);

	_body += "<h1>Index of " + request.uri + "</h1><hr><pre><a href=\"" + url + s + "\">../</a><br>";
	while ((entry = readdir(dir)) != NULL) {
		ss.str("");
		if (ft_strncmp(entry->d_name, ".", 1) != 0 && ft_strncmp(entry->d_name, "..", 2) != 0) {
			if (url[url.length()-1] == '/')
				url = url.substr(0, url.length()-1);
			_body += "<a href=\"" + url + request.uri + entry->d_name + "\">" + entry->d_name + "</a>";
			for (int i = std::string(entry->d_name).length(); i < 51; i++) {
				_body += " ";
			}
			if (stat((path + entry->d_name).c_str(), &stats) == 0) {
				dt = *(gmtime(&stats.st_ctime));
				if (dt.tm_mday < 10)
					ss << "0" << dt.tm_mday << "-";
				else
					ss << dt.tm_mday << "-";
				
				ss	<< months[dt.tm_mon] << "-"
					<< dt.tm_year + 1900 << " "
					<< dt.tm_hour << ":"
					<< dt.tm_min << ":"
					<< dt.tm_sec << "\t\t\t";
				
				if (S_ISDIR(stats.st_mode))
					ss << "-" << "<br>";
				else
					ss << stats.st_size << "<br>";
				_body += ss.str();

			}
		}
	}
	_body += "</pre><hr>";
}

void ResponseHandler::handleBody(request_s& request) {
	int		ret = 1024;
	char	buf[1024];
	int		fd;
	int totalreadsize = 0;
	
	_body.clear();
	if (request.status_code == 400) {
		fd = open(request.server.matchlocation(request.uri).geterrorpage().c_str(), O_RDONLY);
	}
	else {
		fd = generatePage(request);
	}
	if (fd == -3) {
		this->handleAutoIndex(request);		
	}
	else
	{
		while (ret == 1024) {
			ret = read(fd, buf, 1024);
			if (ret <= 0)
				break;
			totalreadsize += ret;
			_body.append(buf, ret);
			memset(buf, 0, 1024);
		}
		if (close(fd) == -1) {
			exit(EXIT_FAILURE);
		}
	}
	size_t pos = _body.find("\r\n\r\n");
	if (request.method == POST) {
		this->extractCGIheaders(_body.substr(0, pos + 4));
		if (pos != std::string::npos)
			_body.erase(0, pos + 4);
	}

}
std::vector<std::string> ResponseHandler::handleRequest(request_s& request) {
	this->_response.resize(1);
	_response.front().clear();
	if (request.method == PUT) {
		handlePut(request);
	}
	else {
		generateResponse(request);
	}
	return _response;
}

void ResponseHandler::handlePut(request_s& request) {
	struct stat statstruct = {};
	_response[0] = "HTTP/1.1 ";

	std::string filePath = request.server.getfilepath(request.uri);
	int statret = stat(filePath.c_str(), &statstruct);

	if (!request.server.matchlocation(request.uri).checkifMethodAllowed(request.method)) {
		_status_code = 405;
		_body.clear();
	}
	else {
		int fd = open(filePath.c_str(), O_TRUNC | O_CREAT | O_WRONLY, S_IRWXU);
		if (fd != -1) {
			if (statret == -1)
				this->_response[0] += _status_codes[201];
			else
				this->_response[0] += _status_codes[204];
			size_t WriteRet = write(fd, request.body.c_str(), request.body.length());
			close(fd);
			if (WriteRet != request.body.length())
				throw std::runtime_error(_RED _BOLD "Write return in ResponseHandler::handlePut is not equal to request.body.length()");
			handleLOCATION(filePath);
		}
		else {
			this->_response[0] += _status_codes[500];
		}
	}
	handleCONNECTION_HEADER(request);
	_response[0] += "\r\n";
}

void ResponseHandler::generateResponse(request_s& request) {
	this->_status_code = 200;
	_response[0] = "HTTP/1.1 ";

	std::vector<Location> v = request.server.getlocations();
	for (size_t i = 0; i < v.size(); i++) {
		std::cout << "Root = " << v[i].getroot() << " Autoindex = " << v[i].getautoindex() << std::endl;
		if (v[i].getautoindex() == "on") {
			this->_autoindex = true;
			this->_autoindex_root = v[i].getroot();
		}
	}
	std::cout << "URI = " << request.uri << " Autoindex = " << request.server.getautoindex() << std::endl;
	if (!request.server.matchlocation(request.uri).checkifMethodAllowed(request.method)) {
		_status_code = 405;
		_body.clear();
	}
	if (this->authenticate(request))
		return;
	if (request.body.length() > request.server.matchlocation(request.uri).getmaxbody()) { // If body length is higher than location::maxBody
		request.status_code = 413;
	}
	if (request.status_code)
		this->_status_code = request.status_code;

	handleBody(request);
	handleStatusCode(request);
	handleCONTENT_TYPE(request);
	handleALLOW();
	handleDATE();
	handleCONTENT_LENGTH();
	handleCONTENT_LOCATION();
	handleCONTENT_LANGUAGE();
	handleSERVER();
	handleCONNECTION_HEADER(request);
	_response[0] += "\r\n";
	if (request.method != HEAD) {
		_response[0] += _body;
		_response[0] += "\r\n";
	}
	_body.clear();
}

int ResponseHandler::authenticate(request_s& request) {
	if (request.server.gethtpasswdpath().empty()) {
		request.headers[AUTHORIZATION].clear();
		return 0;
	}
	std::string username, passwd, str;
	try {
		std::string auth = request.headers.at(AUTHORIZATION);
		std::string type, credentials;
		get_key_value(auth, type, credentials);
		credentials = base64_decode(credentials);
		get_key_value(credentials, username, passwd, ":");
	}
	catch (std::exception& e) {
		std::cerr << "No credentials provided by client" << std::endl;
	}
	request.headers[AUTHORIZATION] = request.headers[AUTHORIZATION].substr(0, request.headers[AUTHORIZATION].find_first_of(' '));
	request.headers[REMOTE_USER] = username;
	if (request.server.getmatch(username, passwd)) {
		std::cout << _GREEN "Authorization successful!" _END << std::endl;
		return 0;
	}

	std::cout << _RED "Authorization failed!" _END << std::endl;
	this->_status_code = 401;
	_response[0] += "401 Unauthorized\r\n";
	this->_response[0] +=	"Server: Webserv/0.1\r\n"
					  	"Content-Type: text/html\r\n"
	   					"WWW-Authenticate: Basic realm=";
	this->_response[0] += request.server.getauthbasicrealm();
	this->_response[0] += ", charset=\"UTF-8\"\r\n";
	return 1;
}

void	ResponseHandler::handleStatusCode(request_s& request) {
	if (this->_status_code == 200 && _cgi_status_code)
		_status_code = _cgi_status_code;
	if (request.version.first != 1 && _status_code == 200)
		_status_code = 505;
	_response[0] += _status_codes[_status_code];
}

void ResponseHandler::handleALLOW() {
	_header_vals[ALLOW] = "GET, HEAD, POST, PUT";
	_response[0] += "Allow: ";
	_response[0] += _header_vals[ALLOW];
	_response[0] += "\r\n";
}

void ResponseHandler::handleCONTENT_LANGUAGE() {
	std::string lang;
	size_t	found = _body.find("<html");
	size_t	lang_idx = _body.find("lang", found + 1);

	if (lang_idx != std::string::npos)
	{
		for (size_t i = lang_idx + 6; _body[i] != '\"'; i++)
			lang += _body[i];
		_header_vals[CONTENT_LANGUAGE] = lang;
	}
	else
	{
		_header_vals[CONTENT_LANGUAGE] = "en-US";
	}
	_response[0] += "Content-Language: ";
	_response[0] += _header_vals[CONTENT_LANGUAGE];
	_response[0] += "\r\n";
}

void ResponseHandler::handleCONTENT_LENGTH() {
	std::stringstream	ss;
	std::string			str;

	_header_vals[CONTENT_LENGTH] = ft::inttostring(_body.length());
	_response[0] += "Content-Length: ";
	_response[0] += _header_vals[CONTENT_LENGTH];
	_response[0] += "\r\n";
}

void ResponseHandler::handleCONTENT_LOCATION() {
	if (!_header_vals[CONTENT_LOCATION].empty()) {
		_response[0] += "Content-Location: ";
		_response[0] += _header_vals[CONTENT_LOCATION];
		_response[0] += "\r\n";
	}
}

void ResponseHandler::handleCONTENT_TYPE(request_s& request) {
	// Defaults to html if no css is found
	if (_cgi_headers.count(CONTENT_TYPE) == 1) {
		_header_vals[CONTENT_TYPE] = _cgi_headers[CONTENT_TYPE];
	}
	if (request.uri.find(".css") != std::string::npos) {
		_header_vals[CONTENT_TYPE] = "text/css";
	}
	else if (request.uri.find(".ico") != std::string::npos) {
		_header_vals[CONTENT_TYPE] = "image/x-icon";
	}
	else if (request.uri.find(".jpg") != std::string::npos || request.uri.find(".jpeg") != std::string::npos) {
		_header_vals[CONTENT_TYPE] = "image/jpeg";
	}
	else {
		_header_vals[CONTENT_TYPE] = "text/html";
	}
	request.headers[CONTENT_TYPE] = this->_header_vals[CONTENT_TYPE];
	_response[0] += "Content-Type: ";
	_response[0] += _header_vals[CONTENT_TYPE];
	_response[0] += "\r\n";
}

void ResponseHandler::handleDATE() {
	_header_vals[DATE] = getCurrentDatetime();
	_response[0] += "Date: ";
	_response[0] += _header_vals[DATE];
	_response[0] += "\r\n";
}

//void ResponseHandler::handleHOST( request_s& request ) {
//	std::stringstream ss;
//	ss << request.server.gethost() << ":" << request.server.getport();
//	_header_vals[HOST] = ss.str();
//	//std::cout << "HOST: " << _header_vals[HOST] << std::endl;
//}

//void ResponseHandler::handleLAST_MODIFIED() {
//	_response[0] += "Last-Modified: ";
//	_response[0] += getCurrentDatetime();
//	_header_vals[LAST_MODIFIED] = getCurrentDatetime();
//	_response[0] += "\r\n";
//}

void ResponseHandler::handleLOCATION( std::string& url ) {
	_header_vals[LOCATION] = url;
	_response[0] += "Location: ";
	_response[0] += url;
	_response[0] += "\r\n";
}

//void ResponseHandler::handleRETRY_AFTER() {
//	_response[0] += "Retry-After: 120\r\n";
//}

void ResponseHandler::handleSERVER() {
	_response[0] += "Server: Webserv/1.0\r\n";
}

void ResponseHandler::handleCONNECTION_HEADER(const request_s& request) {
	if (request.headers.count(CONNECTION) == 1)
		_header_vals[CONNECTION] = request.headers.at(CONNECTION);
	_response[0] += "Connection: " + _header_vals[CONNECTION] + "\r\n";
	// _response[0] += "Accept-Encoding: gzip\r\n";
}

//void ResponseHandler::handleTRANSFER_ENCODING( request_s& request ) {
//	std::stringstream ss;
//	std::string response;
//	int i = 0;
//	_header_vals[TRANSFER_ENCODING] = "chunked";
//	response += "Transfer-Encoding: chunked\r\n\r\n";
//	request.transfer_buffer = true;
//	while (_body.length() > 10000) {
//		ss << std::hex << 10000;
//		response += ss.str();
//		ss.str("");
//		response += "\r\n";
//		response.append(_body, 0, 10000);
//		response += "\r\n";
//		_body.erase(0, 10000);
//		if (i == 0)
//			_response[0] += response;
//		else
//			_response.push_back(response);
//		response.clear();
//		i++;
//	}
//	ss << std::hex << _body.length();
//	response += ss.str();
//	ss.str("");
//	response += "\r\n";
//	response.append(_body, 0, _body.length());
//	_body.clear();
//	response += "\r\n";
//	if (i == 0)
//		_response[0] += response;
//	else
//		_response.push_back(response);
//	response.clear();
//	i++;
//	response += "0\r\n\r\n";
//	_response.push_back(response);
//}
