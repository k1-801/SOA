#include "Output.hpp"

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

void Output::solve(const Input& i)
{
	_sumResult = 0;
	for(double t: i.sums()) _sumResult += t;
	_sumResult *= i.k();
	_mulResult = 1;
	for(int t: i.muls()) _mulResult *= t;
	_sorted = i.sums();
	_sorted.reserve(i.sums().size() + i.muls().size());
	for(int t: i.muls()) _sorted.push_back(t);
	std::sort(_sorted.begin(), _sorted.end());
}

QByteArray Output::toXml()
{
	QDomDocument doc;
	QDomElement root = doc.createElement("Output");
	doc.appendChild(root);
	QDomElement xSumResult = doc.createElement("SumResult");
	QDomElement xMulResult = doc.createElement("MulResult");
	QDomElement xSorted = doc.createElement("SortedInputs");
	root.appendChild(xSumResult);
	root.appendChild(xMulResult);
	root.appendChild(xSorted);
	QDomText srt = doc.createTextNode(QVariant(_sumResult).toString());
	QDomText mrt = doc.createTextNode(QVariant(_mulResult).toString());
	xSumResult.appendChild(srt);
	xMulResult.appendChild(mrt);
	for(double t: _sorted)
	{
		QDomElement sel = doc.createElement("decimal");
		xSorted.appendChild(sel);
		QDomText selt = doc.createTextNode(QVariant(t).toString());
		sel.appendChild(selt);
	}
	QByteArray result = doc.toByteArray();
	// Remove all space symbols
	result.replace(" ", "");
	result.replace("\n", "");
	return result;
}

QByteArray Output::toJson()
{
	QJsonDocument doc;
	QJsonObject root;
	root["SumResult"] = _sumResult;
	root["MulResult"] = _mulResult;
	QJsonArray jsorted;
	for(double t: _sorted) jsorted.push_back(t);
	root["SortedInputs"] = jsorted;
	doc.setObject(root);
	QByteArray result = doc.toJson();
	// Remove all space symbols
	result.replace(" ", "");
	result.replace("\n", "");
	return result;
}

#define TRY_CONVERTING_VECTOR(TEMPLATE,TYPE) \
if(tn.startsWith(#TEMPLATE)) \
{ \
	if(metaproperty.type() == QVariant::fromValue(TEMPLATE<TYPE>()).type()) \
	{ \
		QVariantList container; \
		for(TYPE& val: value.value<TEMPLATE<TYPE>>()) container.push_back(QVariant::fromValue(val)); \
		result[name] = QVariant::fromValue(container); \
	} \
}
#define TRY_CONVERTING_MAP(T,TYPE) \
if(metaproperty.type() == QVariant::fromValue(QMap<QString, TYPE>()).type()) \
{ \
	QMap<QString, TYPE> map = value.value<QMap<QString, TYPE>>(); \
	QVariantMap container; \
	for(auto it = map.begin(); it != map.end(); ++it) container[it.key()] = QVariant::fromValue(it.value()); \
	result[name] = QVariant::fromValue(container); \
}

QVariantMap Output::retrieveFields(const QObject* object)
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
		QVariant value = object->property(name);
		// First: every scalar type can be directly converted to QVariant, no problem
		if(value.canConvert<QString>())
		{
			result[name] = value;
		}
		else
		{
			QByteArray tn = metaproperty.typeName();
			FOR_EACH_TYPE(QVector, TRY_CONVERTING_VECTOR)
			FOR_EACH_TYPE(QList, TRY_CONVERTING_VECTOR)
			FOR_EACH_TYPE(QLinkedList, TRY_CONVERTING_VECTOR)
			FOR_EACH_TYPE(0, TRY_CONVERTING_MAP)
		}
	}
	return result;
}

QByteArray Output::toXml(const QObject* object)
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
		doc.appendChild(el);
	}
	QByteArray result = doc.toByteArray();
	// Remove all space symbols
	result.replace(" ", "");
	result.replace("\n", "");
	return result;
}

QByteArray Output::toJson(const QObject* object)
{
	QJsonDocument doc;
	QJsonObject root;
	QVariantMap map = retrieveFields(object);
	root.fromVariantMap(map);
	QByteArray result = doc.toJson();
	// Remove all space symbols
	result.replace(" ", "");
	result.replace("\n", "");
	return result;
}
