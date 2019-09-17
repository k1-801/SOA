#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdexcept>
using namespace std;

#include "Input.hpp"
#include "Output.hpp"

class Client
{
	private:
		uint16_t port;
		// The following fields are only valid during one connection
		int fd;
		string rec_buffer;

		int send(const std::string& data, int flags)
		{
			return ::send(fd, data.data(), data.size(), flags);
		}

	public:
		Client(uint16_t tport)
		{
			port = tport;
		}

		// Wait until the server returns HTTP OK (200)
		void waitForServer()
		{
			string skip;
			while(request("Ping", skip) != 200);
		}

		int getLine(std::string& line)
		{
			auto it = rec_buffer.find('\n');
			while(it == -1)
			{
				// Not found => receive more data
				char tbuf[1024];
				int size = recv(fd, tbuf, 1024, 0);
				if(size < 0) return -1;
				rec_buffer.append(tbuf, size);
				it = rec_buffer.find('\n');
			}
			// So, we have at least one line in rec_buffer, we need to split the buffer now
			line = rec_buffer.substr(0, it - 1);
			rec_buffer = rec_buffer.substr(it + 1);
			return line.length();
		}

		int getContents(int clen, string& data)
		{
			int left = clen - rec_buffer.size();
			char tbuf[1024];
			while(left)
			{
				int rsize = recv(fd, tbuf, min(left, 1024), 0);
				left -= rsize;
			}
			data.swap(rec_buffer);
			rec_buffer.clear();
			return data.size();
		}

		int request(const std::string& method, std::string& result, const std::string& body = std::string())
		{
			result.clear();
			// TIP: connection is ONLY made to a specific address(127.0.0.1:port), so URL parsing is omitted
			// Every request is made in a separate TCP connection, as the HTTP standard suggests
			// First: create a TCP socket for this particular request
#ifdef DEBUG_OUTPUT
			cout << "===\nRequesting " << method << "\n";
#endif
			fd = socket(AF_INET, SOCK_STREAM, 0);
			if(fd < 0)
			{
				//cout << "connect: failed to create a socket: " << strerror(errno) << "\n";
				return -1;
			}
			struct sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
#ifdef DEBUG
			addr.sin_addr.s_addr = inet_addr("91.245.227.5");
#ifdef DEBUG_OUTPUT
			cout << "On the remote debug host\n";
#endif
#else
			addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#ifdef DEBUG_OUTPUT
			cout << "On localhost:" << port << "\n";
#endif
#endif
			// It took me 37 attempts to find out that connect was failing with ECONNREFUSED, i.e. the server is down

			int succ = 0;
			do
			{
				// wait 1 millisecond for the port to become available
				//struct timespec ts = {0, 1000000};
				//nanosleep(&ts, nullptr);
				sleep(1);
				errno = 0;
				succ = !connect(fd, (struct sockaddr*)&addr, sizeof(addr));
			} while(errno == ECONNREFUSED);
			if(!succ)
			{
				close(fd);
				fd = -1;
				return -1;
			}

#ifdef DEBUG_OUTPUT
			cout << "Connected, making a request to: " << method << "\n";
#endif
			// Great, we have a connection; now we need to form the request
			// It is easier to form an entire request in memory before sending
			ostringstream ostr;
			ostr << (body.empty() ? "GET" : "POST");
#ifdef DEBUG
			ostr << " /study/";
#else
			ostr << " /";
#endif
			ostr << method;
			ostr << " HTTP/1.0\n"
#ifdef DEBUG
				   "Host: localhost\n"
#else
					"Host: 127.0.0.1\n"
#endif
				   "User-agent: k1-801_http_client_dummy\n";
			if(body.size()) ostr << "Content-Length: " << body.size() << "\n";
			ostr << "\n"; // Ends the request
			string req = ostr.str();
			if(send(req, 0) != req.length())
			{
				//cout << "Failed to send the request";
				close(fd);
				fd = -1;
				return -1;
			}
			if(body.size())
			{
				send(body, 0);
			}
			// So, the request is sent, and now we read the answer, line by line
			// The first line will contain the response code

			int ret = -1, clen = -1;
			string line, skip;
			getLine(line);
			istringstream str(line);
			str >> skip >> ret; // We already have a response
#ifdef DEBUG_OUTPUT
			cout << "Response code " << (ret/100 == 2 ? "\e[32m" : "\e[31m") << ret << "\e[0m\n";
#endif
			while(1)
			{
				getLine(line);
				if(line.empty()) break; // The headers end with an empty line, and then the response begins
				istringstream str(line);
				str >> skip;
				if(skip == "Content-Length:") str >> clen;
			}
			if(clen == -1) return -1;
			getContents(clen, result);
#ifdef DEBUG_OUTPUT
			cout << "Content-Length " << clen << "\n---\n\e[32m" << result << "\e[0m\n---\n";
#endif
			close(fd);
			fd = -1;
			return ret;
		}
};

int main()
{
	int port = 80;
#ifndef DEBUG
	//cin >> port;
	port = 20000;
#endif

	Client client(port);
	string task, result;
	client.waitForServer();
	client.request("GetInputData", task);
	Input input;
	Output output;
	input.fromJson(QByteArray::fromStdString(task));
	output.solve(input);
	result = output.toJson().toStdString();
	client.request("WriteAnswer", task, result);
	return 0;
}
