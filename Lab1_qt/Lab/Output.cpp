#include "Output.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QVariant>
#include <QByteArray>

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
