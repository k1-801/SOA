#include "Input.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QVariant>

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

