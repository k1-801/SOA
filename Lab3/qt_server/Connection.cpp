#include "Connection.hpp"

#include "Server.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

Connection::Connection(int tfd, Server* serv)
{
	s = serv;
	fd = tfd;
}
Connection::~Connection()
{
	if(fd != -1)
	{
		close(fd);
		fd = -1;
	}
}

void Connection::cutoff()
{
	buffer = buffer.substr(rlen);
	rlen = 0;
	method.clear();
	protocol.clear();
}

int Connection::respond(const std::string& data, std::map<std::string, std::string>& headers, int code)
{
	std::ostringstream hstream;
	hstream << protocol << code << ' ';
	headers["Server"] = "k1-801_dummy";
	char* buf;
	asprintf(&buf, "%d", data.size());
	headers["Content-Length"] = buf;
	switch(code)
	{
		case 200: hstream << "OK"; break;
		case 201: hstream << "Created"; break;
		case 202: hstream << "Accepted"; break;

		case 403: hstream << "Forbidden"; break;
		case 404: hstream << "Not Found"; break;
		case 405: hstream << "Method Not Allowed"; break;
		case 413: hstream << "Payload Too Large"; break;

		case 500: hstream << "Internal Server Error"; break;
		case 501: hstream << "Not implemented"; break;
	}
	hstream << "\n";
	for(std::pair<const std::string, std::string>& h: headers)
	{
		if(h.first.size())
		{
			hstream << h.first << ": " << h.second << "\n";
		}
	}
	hstream << "\n" << data;
}

void Connection::solve()
{
	// The buffer contains a /PostInputData POST frame that needs to be solved, in JSON
	int hend = buffer.find("\n\n") + 2; // First, find the end of the headers
	s->solve(buffer.data() + hend); // Pass the entire request body in there
}

int Connection::sendAnswer()
{
	const char* answer = s->getAnswer();
	int clen = strlen(answer);
	std::ostringstream hstream;
	hstream << protocol << " 200 OK\n"
			// Possibly add a Date and Last-Modified headers
			"Server: k1-801_dummy\n"
			"Content-Type: text/plain; charset=utf-8\n"
			"Content-Length: " << clen << "\n";
	if(method == "OPTIONS") hstream << "Allow: GET, OPTIONS\n";
	hstream << "Connection: close\n\n";
	std::string headers = hstream.str();
	if(send(fd, headers.data(), headers.length(), 0) != headers.length())
	{
		return 0;
	}
	if(method != "HEAD")
	{
		if(send(fd, answer, clen, 0) != clen)
		{
			return 0;
		}
	}
	return 2;
}

bool Connection::process()
{
	char buf[1024];
	int len, tlen = 0;
	do
	{
		errno = 0;
		len = recv(fd, buf, 1024, 0);
		if(len < 0 && errno != EWOULDBLOCK) return 0; // error -> disconnect immediately
		errno = 0;
		if(len > 0)
		{
			tlen += len;
			buffer.append(buf, len);
		}
	} while(len > 0);
	if(tlen <= 0) // 0 bytes received, but an event was called => close the connection
	{
		return 0;
	}

	int result;
	do
	{
		result = processOneRequest();
		if(!result) return 0;
	} while(result == 2);
	return 1;
}

// returns 2 if processed, 1 if needs more data, 0 on failure -> diconnect immediately;
int Connection::processOneRequest()
{
	int nskip = 0;
	while(buffer[nskip] == '\n' || buffer[nskip] == '\r') ++nskip;
	if(nskip != 0) buffer = buffer.substr(nskip);

	// A call for action! Check ifthe request is complete already
	// First, find out if we have all the headers
	int he = buffer.find("\n\n");
	if(he == -1) he = buffer.find("\r\n\r\n"); // Chrome seems to use \r\n instead of \n
	if(he == -1) return 1; // Nope, need more data

	// Otherwise, we do, so check if we support the method and the page given; basically, split the first line by ' '
	int sp1 = buffer.find(' ');
	int sp2 = buffer.find(' ', sp1 + 1);
	int sp3 = buffer.find('\n', sp2 + 1);
	if(sp1 == -1 || sp2 == -1) return 0; // Something has gone terribly wrong
	method = buffer.substr(0, sp1); // Only GET, POST and HEAD are supported
	std::string page = buffer.substr(sp1 + 1, sp2 - sp1 - 1);
	protocol = buffer.substr(sp2 + 1, sp3 - sp2 - 1); // Will most definetely be HTTP/1.1
	if(protocol[protocol.size() - 1] == '\r') protocol.pop_back();


	int qm = page.find('?');
	if(qm != -1) page.resize(qm); // Brutally cut to the question mark
	for(char& c: page) c = tolower(c);

	// For POST, we also need a Content-length field to be valid, and we need that amount of bytes to be present in buffer; For other methods, we can respond already.
	int clen = -1;
	// Find the Content-length
	int cloff = buffer.find("Content-Length: ");
	if(cloff < he && cloff != -1) // He-he :)
	{
		cloff += 16; // strlen
		int clend = buffer.find('\n', cloff);
		std::string clena = buffer.substr(cloff, clend - cloff);
		clen = atoi(clena.data());
	}
	if(clen > 1000000) // Roughly 1MB
	{
		// Too big -> drop the connection prematurely
#ifdef DEBUG
		cout << "\e[31m413: Payload Too Large (" << clen << " bytes)\e[0m\n";
#endif
		std::ostringstream hstream;
		hstream << protocol << " 413 Payload Too Large\n"
				"Server: k1-801_dummy\n"
				"Content-Type: text/plain; charset=utf-8\n"
				"Content-Length: 0\n"
				"Connection: close\n\n";
		std::string headers = hstream.str();
		if(send(fd, headers.data(), headers.length(), 0) != headers.length())
		{
			return 0;
		}
		return 0;
	}

	// We are sure that at least receving the rest of the request is safe for the server, so now we double-check if it is done already
	rlen = he + clen + (clen == -1); // The total request length
	if(buffer.length() >= rlen)
	{
#ifdef DEBUG
		cout << "A complete request received for page [" << page << "]:\n===\n\e[32m" << buffer << "\e[0m\n===\n";
#endif
		// Yup, we got the request here! Halfway success?
		if(method == "POST" && clen == -1) // not found -> complain
		{
#ifdef DEBUG
			cout << "\e[31m411: Length Required for POST\e[0m\n";
#endif
			std::ostringstream hstream;
			hstream << protocol << " 411 Length Required\n"
					"Server: k1-801_dummy\n"
					"Content-Type: text/plain; charset=utf-8\n"
					"Content-Length: 23\n"
					"Connection: close\n\n"
					"Content-Length required";
			std::string headers = hstream.str();
			if(send(fd, headers.data(), headers.length(), 0) != headers.length())
			{
				return 0;
			}
			return 0; // There's probably a message body following, but we won't see it bc there's no Content-Length, so we just drop the connection
		}

		// Now, check if we support the method given
		if((method != "GET" && method != "POST" && method != "HEAD" && method != "OPTIONS") || (method == "OPTIONS" && page == "*"))
		{
			// Not supported -> respond with 501 "Not implemented"
#ifdef DEBUG
			cout << "\e[31m501: Not Implemented (Method [" << method << "], page [" << page << "])\e[0m\n";
#endif
			std::string reqstring = buffer.substr(0, rlen);
			std::ostringstream hstream;
			hstream << protocol;
			if(method == "OPTIONS") hstream << "200 OK";
			else hstream << " 501 Not implemented";
			hstream << "\n"
					"Server: k1-801_dummy\n"
					"Content-Type: text/plain; charset=utf-8\n"
					"Content-Length: " << rlen + 2 << "\n"
					"Allow: GET, POST, HEAD, OPTIONS\n"
					"Connection: close\n\n[" << reqstring << "]";
			std::string headers = hstream.str();
			if(send(fd, headers.data(), headers.length(), 0) != headers.length())
			{
				return 0;
			}
			cutoff();
			return 1;
		}

		if(page == "/postinputdata" && method == "GET")
		{
			throw 1;
			std::ostringstream hstream;
			hstream << protocol << " 405 Method Not Allowed\n"
					"Allow: POST, HEAD, OPTIONS\n"
					"Server: k1-801_dummy\n"
					"Content-Type: text/plain; charset=utf-8\n"
					"Content-Length: 0\n"
					"Connection: close\n\n";
			std::string headers = hstream.str();
			if(send(fd, headers.data(), headers.length(), 0) != headers.length())
			{
				return 0;
			}
			cutoff();
			return 1;
		}
		else if(page == "/ping" || page == "/stop" || page == "/postinputdata")
		{
			std::ostringstream hstream;
			hstream << protocol << " 200 OK\n"
					"Server: k1-801_dummy\n"
					"Content-Type: text/plain; charset=utf-8\n"
					"Content-Length: 0\n"
					"Connection: close\n\n";
			std::string headers = hstream.str();
			if(send(fd, headers.data(), headers.length(), 0) != headers.length())
			{
				return 0;
			}
			if(page == "/postinputdata")
			{
				std::string body = buffer.substr(he, clen);
				s->solve(body);
			}
			cutoff();
		}
		else if(page == "/getanswer")
		{
			int t = sendAnswer();
			if(t == 2) buffer = buffer.substr(rlen);
			cutoff();
		}
		else
		{
			// Just send 404 Not Found
			std::ostringstream hstream;
			hstream << protocol << " 404 Not Found\n"
					"Server: k1-801_dummy\n"
					"Content-Type: text/plain; charset=utf-8\n"
					"Content-Length: 20\n"
					"Connection: close\n\n"
					"Error 404: Not Found";
			std::string headers = hstream.str();
			if(send(fd, headers.data(), headers.length(), 0) != headers.length())
			{
				return 0;
			}
			cutoff();
		}
		if(page == "/stop")
		{
			s->stop();
		}
	}
	return 2;
}

int Connection::processHeaders(std::map<std::string, std::string>& headers)
{
	// Marks wherethe headers end
	int he = buffer.find("\n\n");
	if(he == -1) he = buffer.find("\r\n\r\n"); // Chrome seems to use \r\n instead of \n
	if(he == -1) return 1; // Nope, need more data
	// Makrs where headers start
	int hs = buffer.find("\n");

}
