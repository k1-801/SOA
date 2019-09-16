#include "Output.hpp"

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

void Output::toJson(std::string& out)
{
	std::ostringstream str;
	// Set the flags so that 30.3 is written as 30.30
	std::ios::fmtflags oldflags = str.setf(std::ios::fixed, std::ios::floatfield);
	std::streamsize oldprec = str.precision(2);
	str << "{\"SumResult\":" << sumResult;
	// Reset flags to their default values
	str.precision(oldprec);
	str.flags(oldflags);
	str << ",\"MulResult\":" << mulResult << ",\"SortedInputs\":[";
	bool k = 0;
	for(double t: sorted)
	{
		if(k) str << ",";
		k = 1;
		str << t;
		if(int(t) == t) str << ".0";
	}
	str << "]}";
	out = str.str();
}
