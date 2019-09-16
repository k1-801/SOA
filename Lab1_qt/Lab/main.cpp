#include <iostream>
#include <QCoreApplication>
#include "Input.hpp"
#include "Output.hpp"

/**
 * Examples:
 * ===
Json
{"K":10,"Sums":[1.01,2.02],"Muls":[1,4]}
 * ===
Xml
<Input><K>10</K><Sums><decimal>1.01</decimal><decimal>2.02</decimal></Sums><Muls><int>1</int><int>4</int></Muls></Input>
 * ===
 */

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	Input input;
	Output output;
	std::string buf, type;
	getline(std::cin, type);
	getline(std::cin, buf);
	QByteArray process(buf.data());
	// Just in case it's out of case
	for(char& c: type) c = tolower(c);
	if(type == "xml")
	{
		input.fromXml(process);
		output.solve(input);
		std::cout << output.toXml().data();
	}
	else
	{
		input.fromJson(process);
		output.solve(input);
		std::cout << output.toJson().data();
	}
	return 0;
}
