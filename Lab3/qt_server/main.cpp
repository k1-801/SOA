#include <iostream>

#include "Server.hpp"

int main()
{
	int port;
	std::cin >> port;
	Server s;
	s.exec(port);
	return 0;
}
