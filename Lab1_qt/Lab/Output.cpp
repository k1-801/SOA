#include "Output.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QVariant>

void Output::solve(const Input& i)
{
	sumResult = 0;
	for(double t: i.sums) sumResult += t;
	sumResult *= i.k;
	mulResult = 1;
	for(int t: i.muls) mulResult *= t;
	sorted = i.sums;
	sorted.reserve(i.sums.size() + i.muls.size());
	for(int t: i.muls) sorted.push_back(t);
	std::sort(sorted.begin(), sorted.end());
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
	QDomText srt = doc.createTextNode(QVariant(sumResult).toString());
	QDomText mrt = doc.createTextNode(QVariant(mulResult).toString());
	xSumResult.appendChild(srt);
	xMulResult.appendChild(mrt);
	for(double t: sorted)
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
	root["SumResult"] = sumResult;
	root["MulResult"] = mulResult;
	QJsonArray jsorted;
	for(double t: sorted) jsorted.push_back(t);
	root["SortedInputs"] = jsorted;
	doc.setObject(root);
	QByteArray result = doc.toJson();
	// Remove all space symbols
	result.replace(" ", "");
	result.replace("\n", "");
	return result;
}
