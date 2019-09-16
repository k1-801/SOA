#ifndef INPUT_HPP
#define INPUT_HPP

#include <QVector>
#include <QString>
#include <sstream>

class Input
{
	public:
		int k;
                QVector<double> sums;
                QVector<int> muls;
                void fromJson(const QString& in);
};

#endif // INPUT_HPP
