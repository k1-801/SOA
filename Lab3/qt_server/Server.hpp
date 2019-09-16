#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <memory>

class Connection;
typedef std::shared_ptr<Connection> PConnection;
typedef std::weak_ptr<Connection> WConnection;

class Server
{
	private:
		uint16_t port;
		int fd = -1; // Listening socket
		int epollfd = -1;
		std::string answer; // For performance reasons, I store it as a result string. I don't want to repack theentire solution every time it is requested!
		bool _run;
		std::map<int, PConnection> conns;

		bool prepare();

	public:
		Server();
		bool exec(uint16_t tport);
		void solve(const std::string& in);
		const char* getAnswer();
		void stop();
};
#endif // SERVER_HPP
