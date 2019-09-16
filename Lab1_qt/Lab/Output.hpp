#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <QByteArray>
#include <QVector>

class Output;

#include "Input.hpp"

class Output
{
	public:
		double sumResult;
		int mulResult;
		QVector<double> sorted;
		void solve(const Input& i);
		QByteArray toXml();
		QByteArray toJson();
};

#endif // OUTPUT_HPP
