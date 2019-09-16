#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <memory>
#include <string>
#include <map>

class Connection;
class Server;
typedef std::shared_ptr<Connection> PConnection;
typedef std::weak_ptr<Connection> WConnection;

class Connection : std::enable_shared_from_this<Connection>
{
	private:
		Server* s = nullptr;
		int fd = -1;
		std::string buffer; // Whatever comes to be read from the socket, stays here until the request is complete
		int rlen; // Request length
		// Request info - possibly move to a separate class
		std::string method; // GET, POST, HEAD, partially OPTIONS
		std::string protocol;
		void cutoff();
		void solve();
		int processOneRequest();
		int processHeaders(std::map<std::string, std::string>& headers);
		int respond(const std::string& data, std::map<std::string, std::string>& headers, int code = 200);
		int respond(const std::string& data, int code = 200);

	public:
		Connection(int tfd, Server* serv);
		~Connection();

		// An event occured;
		// returned 0 means that the connection must be deleted
		bool process();
		int sendAnswer();
};

#endif // CONNECTION_HPP
