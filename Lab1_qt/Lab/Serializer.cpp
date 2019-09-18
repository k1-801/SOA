#include "Serializer.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QVariant>
#include <QString>
#include <QVector>
#include <QList>
#include <QLinkedList>
#include <QMetaProperty>
#include <QVarLengthArray>

#define FOR_EACH_TYPE(TEMPLATE,F) \
if(tn.startsWith(#TEMPLATE)) \
{ \
	F(TEMPLATE, bool) \
	F(TEMPLATE, int) \
	F(TEMPLATE, uint) \
	F(TEMPLATE, qlonglong) \
	F(TEMPLATE, qulonglong) \
	F(TEMPLATE, double) \
	F(TEMPLATE, long) \
	F(TEMPLATE, short) \
	F(TEMPLATE, char) \
	F(TEMPLATE, ulong) \
	F(TEMPLATE, ushort) \
	F(TEMPLATE, uchar) \
	F(TEMPLATE, float) \
	F(TEMPLATE, signed char) \
	F(TEMPLATE, std::nullptr_t) \
	F(TEMPLATE, QCborSimpleType) \
	F(TEMPLATE, QString) \
	{} \
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


void Serializer::fillFields(const QVariantMap& in, QObject* object)
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

void Serializer::fromXml(const QByteArray& in, QObject* object)
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

void Serializer::fromJson(const QByteArray& in, QObject* object)
{
	QJsonDocument doc;
	QJsonParseError er;
	doc = QJsonDocument::fromJson(in, &er);
	if(doc.isNull()) return; // Error
	QVariant var = doc.toVariant();
	QJsonObject root = doc.object();
	fillFields(root.toVariantMap(), object);
}


#define TRY_CONVERTING_TO_VECTOR(TEMPLATE,TYPE) \
if(tn == #TEMPLATE "<" #TYPE ">") /*if(metaproperty.type() == QVariant::fromValue(TEMPLATE<TYPE>()).type())*/ \
{ \
	QVariantList container; \
	for(TYPE& val: value.value<TEMPLATE<TYPE>>()) container.push_back(QVariant::fromValue(val)); \
	result[name] = QVariant::fromValue(container); \
} else

#define TRY_CONVERTING_TO_MAP(TEMPLATE,TYPE) \
if(tn == #TEMPLATE "<QString," #TYPE ">") /*if(metaproperty.type() == QVariant::fromValue(QMap<QString, TYPE>()).type())*/ \
{ \
	QMap<QString, TYPE> map = value.value<QMap<QString, TYPE>>(); \
	QVariantMap container; \
	for(auto it = map.begin(); it != map.end(); ++it) container[it.key()] = QVariant::fromValue(it.value()); \
	result[name] = QVariant::fromValue(container); \
} else

QVariantMap Serializer::retrieveFields(const QObject* object)
{
	// Convert everything to QVariant
	// Every QVector, QList, QLinkedList to QVariantList
	// Every QMap to QVariantMap
	QVariantMap result;
	const QMetaObject *metaobject = object->metaObject();
	int count = metaobject->propertyCount();
	for (int i = 0; i < count; ++i)
	{
		QMetaProperty metaproperty = metaobject->property(i);
		const char* name = metaproperty.name();
		if(!strcmp(name, "objectName")) continue;
		QVariant value = object->property(name);
		// First: every scalar type can be directly converted to QVariant, no problem
		if(value.canConvert<QString>())
		{
			result[name] = value;
		}
		else
		{
			QByteArray tn = metaproperty.typeName();
			FOR_EACH_TYPE(QVector, TRY_CONVERTING_TO_VECTOR)
			FOR_EACH_TYPE(QList, TRY_CONVERTING_TO_VECTOR)
			FOR_EACH_TYPE(QLinkedList, TRY_CONVERTING_TO_VECTOR)
			FOR_EACH_TYPE(QMap, TRY_CONVERTING_TO_MAP)
		}
	}
	return result;
}

QByteArray Serializer::toXml(const QObject* object)
{
	QDomDocument doc;
	QDomElement root = doc.createElement("Output");
	doc.appendChild(root);
	QVariantMap map = retrieveFields(object);
	for(auto it = map.begin(); it != map.end(); ++it)
	{
		QDomElement el = doc.createElement(it.key());
		QVariant value = it.value();
		if(value.canConvert<QString>())
		{
			QDomText text = doc.createTextNode(value.toString());
			el.appendChild(text);
		}
		else if(value.canConvert<QVariantList>())
		{
			QVariantList list = value.value<QVariantList>();
			for(QVariant& list_el: list)
			{
				QString subtype = list_el.typeName();
				if(subtype == "double") subtype = "decimal"; // Yeah, C# fans, I hear ya
				QDomElement subel = doc.createElement(subtype);
				el.appendChild(subel);
				QDomText text = doc.createTextNode(list_el.toString());
				// This is gonna work
				// Because I don't know what I'm gonna do if it doesn't
				// (c) Cap
				subel.appendChild(text);
			}
		}
		else if(value.canConvert<QVariantMap>())
		{
			QVariantMap nmap = value.value<QVariantMap>();
			for(auto it = nmap.begin(); it != nmap.end(); ++it)
			{
				QDomElement subel = doc.createElement(it.key());
				el.appendChild(subel);
				QDomText text = doc.createTextNode(it.value().toString());
				// Same as above: just have to hope that it isn't too complicated
				// Otherwise it'd require a RECURSIVE function here
				subel.appendChild(text);
			}
		}
		// I am pretty sure all the other options are impossible
		root.appendChild(el);
	}
	QByteArray result = doc.toByteArray();
	// Remove all space symbols
	result.replace(" ", "");
	result.replace("\n", "");
	return result;
}

QByteArray Serializer::toJson(const QObject* object)
{
	QJsonDocument doc;
	QVariantMap map = retrieveFields(object);
	QJsonObject root = QJsonObject::fromVariantMap(map);
	doc.setObject(root);
	QByteArray result = doc.toJson();
	// Remove all space symbols
	result.replace(" ", "");
	result.replace("\n", "");
	return result;
}
