#include <iostream>
#include <QCoreApplication>
#include <QHttpServer>
#include "Input.hpp"
#include "Output.hpp"

int main(int argc, char** argv)
{
	QCoreApplication app(argc, argv);
	QHttpServer server;
	Input input;
	Output output;

	int port;
	std::cin >> port;
	server.listen(QHostAddress::Any, port);
	server.route("Ping",[](){return QHttpServerResponse::StatusCode::Ok;});
	server.route("Stop",[]()
	{
		QCoreApplication::quit();
		return QHttpServerResponse::StatusCode::Ok;
	});
	server.route("PostInputData",[&](const QHttpServerRequest& req)
	{
		input.fromJson(req.body());
		output.solve(input);
		return QHttpServerResponse::StatusCode::Ok;
	});
	server.route("GetAnswer",[&]()
	{
		return output.toJson();
	});
	return app.exec();
}
