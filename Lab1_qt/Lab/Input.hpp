#ifndef INPUT_HPP
#define INPUT_HPP

#include <QVector>
#include <QString>

class Input
{
	public:
		int k;
		QVector<double> sums;
		QVector<int> muls;
		void fromXml(const QByteArray& in);
		void fromJson(const QByteArray& in);
};

#endif // INPUT_HPP
