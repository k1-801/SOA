#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include <QByteArray>
#include <QObject>

class Serializer
{
	private: // We don't need the actual INSANCES of this class to be made
		Serializer(){}
		~Serializer(){}

		static void fillFields(const QVariantMap& in, QObject* object);
		static QVariantMap retrieveFields(const QObject* object);

	public:
		static void fromXml(const QByteArray& in, QObject*);
		static void fromJson(const QByteArray& in, QObject*);
		static QByteArray toXml(const QObject*);
		static QByteArray toJson(const QObject*);
};

#endif // SERIALIZER_HPP
