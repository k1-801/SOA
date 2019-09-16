#include "Server.hpp"
#include "Connection.hpp"
#include "Input.hpp"
#include "Output.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

Server::Server()
{

}

bool Server::prepare()
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

bool Server::exec(uint16_t tport)
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
				conns[new_fd] = std::make_shared<Connection>(new_fd, this);
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

void Server::solve(const std::string& in)
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

const char* Server::getAnswer()
{
	return answer.data();
}

void Server::stop()
{
	_run = 0;
}
