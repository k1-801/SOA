#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <QByteArray>
#include <QVector>

class Output;

#include "Input.hpp"

class Output : public QObject
{
		Q_OBJECT
		Q_PROPERTY(double SumResult READ sumResult WRITE setSumResult)
		Q_PROPERTY(int MulResult READ mulResult WRITE setMulResult)
		Q_PROPERTY(QVector<double> SortedInputs READ sorted WRITE setSorted)

	private:
		double _sumResult;
		int _mulResult;
		QVector<double> _sorted;
		static QVariantMap retrieveFields(const QObject*);

	public:
		Output(){}
		~Output(){}
		void setSumResult(double sumResult){_sumResult = sumResult;}
		void setMulResult(int mulResult){_mulResult = mulResult;}
		void setSorted(const QVector<double>& sorted){_sorted = sorted;}
		double sumResult(){return _sumResult;}
		int mulResult(){return _mulResult;}
		QVector<double> sorted(){return _sorted;}

		void solve(const Input& i);
		QByteArray toXml();
		QByteArray toJson();
		static QByteArray toXml(const QObject*);
		static QByteArray toJson(const QObject*);
};

#endif // OUTPUT_HPP
