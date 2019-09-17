#ifndef INPUT_HPP
#define INPUT_HPP

#include <QVector>
#include <QString>
#include <QObject>

class Input : public QObject
{
		Q_OBJECT
		Q_PROPERTY(int K READ k WRITE setK)
		Q_PROPERTY(QVector<double> Sums READ sums WRITE setSums)
		Q_PROPERTY(QVector<int> Muls READ muls WRITE setMuls)

		int _k;
		QVector<double> _sums;
		QVector<int> _muls;
		static void fillFields(const QVariantMap& in, QObject*);

	public:
		Input(){}
		~Input(){}
		void setK(int k){_k = k;}
		void setSums(const QVector<double>& sums){_sums = sums;}
		void setMuls(const QVector<int>& muls){_muls = muls;}
		int k() const {return _k;}
		QVector<double> sums() const {return _sums;}
		QVector<int> muls() const {return _muls;}

		void fromXml(const QByteArray& in);
		void fromJson(const QByteArray& in);
		static void fromXml(const QByteArray& in, QObject*);
		static void fromJson(const QByteArray& in, QObject*);
};

#endif // INPUT_HPP
