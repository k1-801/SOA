#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <list>
#include <map>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <fcntl.h>
using namespace std;

// IMPORTANT! Does NOT support Multipart POST requests, so no file uploads!
// NOTE: connections with no interaction on them will be left open until the server stops; there is no timeout

//#define SCHEDULE

struct Input
{
		int k;
		vector<double> sums;
		vector<int> muls;
		void fromJson(const std::string& in)
		{
			istringstream str(in);
			char skip;
			// Just in case you wanna run this in a loop; like in a server, maybe?
			sums.clear();
			muls.clear();
			do{str >> skip;} while(skip != 'K');
			str >> skip >> skip; // skip a "\":"
			str >> k; // First field
			do{str >> skip;} while(skip != '['); // Move to where the array begins
			do // Read Sums; this WILL fail if the array is empty, this scenario requires MANUAL numbers parsing too
			{
				sums.push_back(0);
				str >> sums[sums.size() - 1];
				str >> skip; // Read a "," OR a "]"
			} while(skip != ']');
			do{str >> skip;} while(skip != '['); // Move to where the array begins
			do // Read Muls; this WILL fail if the array is empty, this scenario requires MANUAL numbers parsing too
			{
				muls.push_back(0);
				str >> muls[muls.size() - 1];
				str >> skip; // Read a "," OR a "]"
			} while(skip != ']');
		}
};

struct Output
{
		double sumResult;
		int mulResult;
		vector<double> sorted;
		void solve(const Input& i)
		{
			sumResult = 0;
			for(double t: i.sums) sumResult += t;
			sumResult *= i.k;
			mulResult = 1;
			for(int t: i.muls) mulResult *= t;
			sorted = i.sums;
			sorted.reserve(i.sums.size() + i.muls.size());
			for(int t: i.muls) sorted.push_back(t);
			sort(sorted.begin(), sorted.end());
		}
		void toJson(std::string& out)
		{
			ostringstream str;
			// Set the flags so that 30.3 is written as 30.30
			ios::fmtflags oldflags = str.setf(ios::fixed, ios::floatfield);
			streamsize oldprec = str.precision(2);
			str << "{\"SumResult\":" << sumResult;
			// Reset flags to their default values
			str.precision(oldprec);
			str.flags(oldflags);
			str << ",\"MulResult\":" << mulResult << ",\"SortedInputs\":[";
			bool k = 0;
			for(double t: sorted)
			{
				if(k) str << ",";
				k = 1;
				str << t;
				if(int(t) == t) str << ".0";
			}
			str << "]}";
			out = str.str();
		}
};

class Server;
// I DO have a class containing the full connection info. It is REQUIRED to store the I/O buffers.
class Connection : enable_shared_from_this<Connection>
{
	private:
		Server* s = nullptr;
		int fd = -1;
		string buffer; // Whatever comes to be read from the socket, stays here until the request is complete
		int rlen; // Request length
		// Request info - possibly move to a separate class
		string method; // GET, POST, HEAD, partially OPTIONS
		string protocol;
		void cutoff()
		{
			buffer = buffer.substr(rlen);
			rlen = 0;
			method.clear();
			protocol.clear();
		}
		void solve();
		int processOneRequest();

	public:
		Connection(int tfd, Server* serv)
		{
			s = serv;
			fd = tfd;
#ifdef DEBUG
			cout << "New connection [" << tfd << "]\n";
#endif
		}
		~Connection()
		{
			if(fd != -1)
			{
				close(fd);
				fd = -1;
			}
		}

		// An event occured;
		// returned 0 means that the connection must be deleted
		bool process();
		int sendAnswer();
};
typedef std::shared_ptr<Connection> PConnection;
typedef std::weak_ptr<Connection> WConnection;

class Server
{
	private:
		uint16_t port;
		int fd = -1; // Listening socket
		int epollfd = -1;
		string answer; // For performance reasons, I store it as a result string. I don't want to repack theentire solution every time it is requested!
		bool _run;
		map<int, PConnection> conns;
		list<WConnection> scheduled;

		bool prepare()
		{
			fd = socket(AF_INET, SOCK_STREAM, 0);
			epollfd = epoll_create(102);
			if(fd < 0 || epollfd < 0)
			{
#ifdef DEBUG
				perror("Failed to create socket");
#endif
				return 0;
			}
			struct sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
			{
#ifdef DEBUG
				perror("Failed to bind socket");
#endif
				return 0;
			}
			if(::listen(fd, 1))
			{
				return 0;
			}
			// Add the main socket to the EPOLL list so that we cn wait until a new connection shows up
			static struct epoll_event ev;
			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = fd;
			if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev))
			{
				return 0;
			}
			return 1;
		}

	public:
		Server(){}

		bool exec(uint16_t tport)
		{
			port = tport;
			if(!prepare()) return 0;
			static struct epoll_event events[100];
			_run = 1;
			while(_run)
			{
				// Work with the sockets
				int nfds = epoll_wait(epollfd, events, 100, 1);
				if(nfds < 0) return 0;
				for(int i = 0; i < nfds; i++)
				{
#ifdef DEBUG
				if(nfds) cout << "Event on " << events[i].data.fd << "\n";
#endif
					int event_fd = events[i].data.fd;
					if(event_fd == fd)
					{
						// Accept a new connection
						int new_fd = accept(fd, nullptr, nullptr);
						static struct epoll_event ev;
						ev.events = EPOLLIN | EPOLLET;
						ev.data.fd = new_fd;
						if(epoll_ctl(epollfd, EPOLL_CTL_ADD, new_fd, &ev))
						{
							// Okay, this IS an error, but we can't stop the server!
						}
						// Make the socket non-blocking so that we don't have to wait for new data
						fcntl(new_fd, F_SETFL, fcntl (new_fd, F_GETFL, 0) | O_NONBLOCK);
						conns[new_fd] = make_shared<Connection>(new_fd, this);
					}
					else
					{
						// Find the respective connection and process it
						PConnection tconn = conns[event_fd];
						if(!tconn->process())
						{
							// If the processing says to drop this connection, just drop it.
							conns.erase(event_fd);
						}
					}
				}
			}
			// Clean up everything
			if(fd != -1) close(fd);
			if(epollfd != -1) close(epollfd);
			return 1;
		}

		void solve(const string& in)
		{
			Input input;
			Output output;
			input.fromJson(in);
			output.solve(input);
			output.toJson(answer);
#ifdef SCHEDULE
			sendToScheduled();
#endif
		}

		const char* getAnswer()
		{
			return answer.data();
		}
#ifdef SCHEDULE
		void scheduleResponse(PConnection conn)
		{
			scheduled.push_back(conn);
		}

		void sendToScheduled()
		{
			for(WConnection wc: scheduled)
			{
				PConnection pc = wc.lock();
				if(pc)
				{
					pc->sendAnswer();
				}
			}
			scheduled.clear();
		}
#endif

		void stop()
		{
			_run = 0;
		}
};

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
#ifdef SCHEDULE
	if(!clen)
	{
		s->scheduleResponse(shared_from_this());
		return 1;
	}
#endif
	ostringstream hstream;
	hstream << protocol << " 200 OK\n"
			// Possibly add a Date and Last-Modified headers
			"Server: k1-801_dummy\n"
			"Content-Type: text/plain; charset=utf-8\n"
			"Content-Length: " << clen << "\n";
	if(method == "OPTIONS") hstream << "Allow: GET, OPTIONS\n";
	hstream << "Connection: close\n\n";
	string headers = hstream.str();
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
	if(he == -1)
	{
		he = buffer.find("\r\n\r\n");
		if(he == -1)
		{
			cout << "\e[33mNeed more data: headers end not found\e[0m\n";
			return 1; // Nope, need more data
		}
		he += 4;
	}
	else
	{
		he += 2;
	}

	// Otherwise, we do, so check if we support the method and the page given; basically, split the first line by ' '
	int sp1 = buffer.find(' ');
	int sp2 = buffer.find(' ', sp1 + 1);
	int sp3 = buffer.find('\n', sp2 + 1);
	if(sp1 == -1 || sp2 == -1) return 0; // Something has gone terribly wrong
	method = buffer.substr(0, sp1); // Only GET, POST and HEAD are supported
	string page = buffer.substr(sp1 + 1, sp2 - sp1 - 1);
	protocol = buffer.substr(sp2 + 1, sp3 - sp2 - 1); // Will most definetely be HTTP/1.1
	if(protocol[protocol.size() - 1] == '\r') protocol.pop_back();


	int qm = page.find('?');
	if(qm != -1) page.resize(qm); // Brutally cut to the question mark
	for(char& c: page) c = tolower(c);

	// Try to process all headers
	string head_string = buffer.substr(0, he);
	map<string, string> headers;


	// For POST, we also need a Content-length field to be valid, and we need that amount of bytes to be present in buffer; For other methods, we can respond already.
	int clen = -1;
	// Find the Content-length
	int cloff = head_string.find("Content-Length: ");
	if(cloff < he && cloff != -1) // He-he :)
	{
		cloff += 16; // strlen
		int clend = head_string.find('\n', cloff);
		string clena = head_string.substr(cloff, clend - cloff);
		clen = atoi(clena.data());
	}
	if(clen > 1000000) // Roughly 1MB
	{
		// Too big -> drop the connection prematurely
#ifdef DEBUG
		cout << "\e[31m413: Payload Too Large (" << clen << " bytes)\e[0m\n";
#endif
		ostringstream hstream;
		hstream << protocol << " 413 Payload Too Large\n"
				"Server: k1-801_dummy\n"
				"Content-Type: text/plain; charset=utf-8\n"
				"Content-Length: 0\n"
				"Connection: close\n\n";
		string headers = hstream.str();
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
			ostringstream hstream;
			hstream << protocol << " 411 Length Required\n"
					"Server: k1-801_dummy\n"
					"Content-Type: text/plain; charset=utf-8\n"
					"Content-Length: 23\n"
					"Connection: close\n\n"
					"Content-Length required";
			string headers = hstream.str();
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
			string reqstring = buffer.substr(0, rlen);
			ostringstream hstream;
			hstream << protocol;
			if(method == "OPTIONS") hstream << "200 OK";
			else hstream << " 501 Not implemented";
			hstream << "\n"
					"Server: k1-801_dummy\n"
					"Content-Type: text/plain; charset=utf-8\n"
					"Content-Length: " << rlen + 2 << "\n"
					"Allow: GET, POST, HEAD, OPTIONS\n"
					"Connection: close\n\n[" << reqstring << "]";
			string headers = hstream.str();
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
			ostringstream hstream;
			hstream << protocol << " 405 Method Not Allowed\n"
					"Allow: POST, HEAD, OPTIONS\n"
					"Server: k1-801_dummy\n"
					"Content-Type: text/plain; charset=utf-8\n"
					"Content-Length: 0\n"
					"Connection: close\n\n";
			string headers = hstream.str();
			if(send(fd, headers.data(), headers.length(), 0) != headers.length())
			{
				return 0;
			}
			cutoff();
			return 1;
		}
		else if(page == "/ping" || page == "/stop" || page == "/postinputdata")
		{
			ostringstream hstream;
			hstream << protocol << " 200 OK\n"
					"Server: k1-801_dummy\n"
					"Content-Type: text/plain; charset=utf-8\n"
					"Content-Length: 0\n"
					"Connection: close\n\n";
			string headers = hstream.str();
			if(send(fd, headers.data(), headers.length(), 0) != headers.length())
			{
				return 0;
			}
			if(page == "/postinputdata")
			{
				string body = buffer.substr(he, clen);
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
			ostringstream hstream;
			hstream << protocol << " 404 Not Found\n"
					"Server: k1-801_dummy\n"
					"Content-Type: text/plain; charset=utf-8\n"
					"Content-Length: 20\n"
					"Connection: close\n\n"
					"Error 404: Not Found";
			string headers = hstream.str();
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

int main()
{
	int port;
	cin >> port;
	Server s;
	s.exec(port);
	return 0;
}
