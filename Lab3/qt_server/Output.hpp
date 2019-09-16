#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

class Output;

#include "Input.hpp"

class Output
{
	public:
		double sumResult;
		int mulResult;
		std::vector<double> sorted;
		void solve(const Input& i);
		void toJson(std::string& out);
};

#endif // OUTPUT_HPP
