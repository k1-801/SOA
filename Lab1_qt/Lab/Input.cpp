#include "Input.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QVariant>
#include <QLinkedList>
#include <QList>
#include <QVector>
#include <QVarLengthArray>

#include <QMetaProperty>

void Input::fromXml(const QByteArray& in)
{
	QDomDocument doc;
	doc.setContent(in);
	QDomElement root = doc.documentElement();

	// Check tag validity
	if(root.tagName() != "Input") return;
	QDomNodeList knl = root.elementsByTagName("K");
	QDomNodeList snl = root.elementsByTagName("Sums");
	QDomNodeList mnl = root.elementsByTagName("Muls");
	if(knl.size() != 1 || snl.size() != 1 || mnl.size() != 1) return;

	// Parse
	_k = QVariant(knl.at(0).firstChild().toText().data()).toInt();
	snl = snl.at(0).childNodes();
	mnl = mnl.at(0).childNodes();
	_sums.resize(snl.size());
	_muls.resize(mnl.size());
	for(int i = 0; i < snl.size(); ++i)
	{
		_sums[i] = QVariant(snl.at(i).firstChild().toText().data()).toDouble();
	}
	for(int i = 0; i < mnl.size(); ++i)
	{
		_muls[i] = QVariant(mnl.at(i).firstChild().toText().data()).toDouble();
	}
}

void Input::fromJson(const QByteArray& in)
{
	_sums.clear();
	_muls.clear();
	QJsonDocument doc;
	QJsonParseError er;
	doc = QJsonDocument::fromJson(in, &er);
	if(doc.isNull()) return; // Error
	QJsonObject root = doc.object();
	_k = root["K"].toInt();
	QJsonArray jsums = root["Sums"].toArray();
	QJsonArray jmuls = root["Muls"].toArray();
	for(const QJsonValue t: jsums) _sums.push_back(t.toDouble());
	for(const QJsonValue t: jmuls) _muls.push_back(t.toInt());
}

#define TRY_CONVERTING_VECTOR(TEMPLATE,TYPE) \
if(tn == #TEMPLATE "<" #TYPE ">") /*if(metaproperty.type() == QVariant::fromValue(TEMPLATE<TYPE>()).type())*/ \
{ \
	TEMPLATE<TYPE> container; \
	for(QVariant val: value.toList()) container.push_back(val.value<TYPE>()); \
	object->setProperty(name, QVariant::fromValue(container)); \
} else

#define TRY_CONVERTING_MAP(TEMPLATE,TYPE) \
if(tn == #TEMPLATE "<QString," #TYPE ">") /*if(metaproperty.type() == QVariant::fromValue(QMap<QString, TYPE>()).type())*/ \
{ \
	QMap<QString, TYPE> container; \
	QVariantMap map = value.toMap(); \
	for(auto it = map.begin(); it != map.end(); ++it) container[it.key()] = it.value().value<TYPE>(); \
	object->setProperty(name, QVariant::fromValue(container)); \
} else


void Input::fillFields(const QVariantMap& in, QObject* object)
{
	const QMetaObject *metaobject = object->metaObject();
	int count = metaobject->propertyCount();
	for (int i = 0; i < count; ++i)
	{
		QMetaProperty metaproperty = metaobject->property(i);
		const char* name = metaproperty.name();
		QVariant value = in.value(name);
		QVariant target = object->property(name);
		if(in.value(name).canConvert(metaproperty.type()))
		{
			object->setProperty(name, value);
		}
		else
		{
			QByteArray tn = metaproperty.typeName();
			// Cannot convert directly: maps, vectors, lists
			if(value.canConvert<QVariantList>())
			{
				// vectors, lists: iteratable structures
				// templates
				FOR_EACH_TYPE(QVector, TRY_CONVERTING_VECTOR)
				FOR_EACH_TYPE(QList, TRY_CONVERTING_VECTOR)
				FOR_EACH_TYPE(QLinkedList, TRY_CONVERTING_VECTOR)
				//FOR_EACH_STATIC_PRIMITIVE_TYPE(QVarLengthArray, TRY_CONVERTING_VECTOR)
			}
			if(value.canConvert<QVariantMap>())
			{
				// Either a map or a custom class; I can't fill NESTED classes, but I can try to fill a map.
				// Large TODO: this will take too long to finish
				FOR_EACH_TYPE(QMap, TRY_CONVERTING_MAP)
			}
		}
	}
}

void Input::fromXml(const QByteArray& in, QObject* object)
{
	// Note: this one will only fill the EXISTING QObject's properties
	// It does not create extra properties for extraneous tags
	// Also, if a tag is present more than once, only the first occasion will be processed
	QDomDocument doc;
	doc.setContent(in);
	QDomElement root = doc.documentElement();
	if(root.tagName() != "Input") return;
	auto els1 = root.childNodes();

	QVariantMap map;
	for(int i = 0; i < els1.size(); ++i)
	{
		// First level children: class fields themselves
		auto node1 = els1.at(i);
		if(node1.isElement())
		{
			auto el1 = node1.toElement();
			QVariant val;
			QString tag1 = el1.tagName();
			auto els2 = el1.childNodes();
			QVector<QPair<QString, QVariant>> subvals;
			bool sameType = 1;
			for(int j = 0; j < els2.size(); ++j)
			{
				auto node2 = els2.at(j);
				if(node2.isText())
				{
					// Note: if there's more than ne text... I don't know what to do
					val = node2.toText().data();
				}
				else if(node2.isElement())
				{
					auto el2 = node2.toElement();
					// Pretty much HAS to contain text ONLY, otherwise the parser would hve to be recursive and I DON'T want to write that shit
					QVariant subval;
					auto els3 = el2.childNodes();
					for(int k = 0; k < els3.size(); ++k)
					{
						auto node3 = els3.at(k);
						if(node3.isText())
						{
							subval = node3.toText().data();
						}
					}
					QString tag = el2.tagName();
					if(subvals.size() && subvals.last().first != tag)
						sameType = 0;
					subvals.push_back(QPair<QString, QVariant>(tag, subval));
				}
			}

			if(subvals.size())
			{
				if(sameType)
				{
					// List
					QVariantList list;
					for(auto nv: subvals) list.append(nv.second);
					val = list;
				}
				else
				{
					// Map
					QVariantMap map;
					for(auto nv: subvals) map[nv.first] = nv.second;
					val = map;
				}
			}
			map[tag1] = val;
		}
	}
	fillFields(map, object);
}

void Input::fromJson(const QByteArray& in, QObject* object)
{
	QJsonDocument doc;
	QJsonParseError er;
	doc = QJsonDocument::fromJson(in, &er);
	if(doc.isNull()) return; // Error
	QVariant var = doc.toVariant();
	QJsonObject root = doc.object();
	fillFields(root.toVariantMap(), object);
}
