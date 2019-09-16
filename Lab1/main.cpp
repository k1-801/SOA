#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
using namespace std;

/**
 * NOTE: Deserializer heavily relies on the structures being EXACTLY like in the task, with no extra spaces, and each array containing at least one member, i.e. not empty
 * Examples:
 * ===
json
{"K":10,"Sums":[1.01,2.02],"Muls":[1,4]}
 * ===
xml
<Input><K>10</K><Sums><decimal>1.01</decimal><decimal>2.02</decimal></Sums><Muls><int>1</int><int>4</int></Muls></Input>
 * ===
 */
struct Input
{
	int k;
	vector<double> sums;
	vector<int> muls;
	void fromXml(const std::string& in)
	{
		istringstream str(in);
		char skip;
		// Just in case you wanna run this in a loop; like in a server, maybe?
		sums.clear();
		muls.clear();
		do{str >> skip;} while(skip != 'K');
		str >> skip; // skip a ">"
		str >> k; // First field
		while(1) // Read Sums
		{
			do{str >> skip;} while(skip != 'l'); // Search the 'l' in "decimal" OR in "muls" and check exactly which one is that
			str >> skip;
			if(skip == '>') // decimal => a member of "sums"
			{
				sums.push_back(0);
				str >> sums[sums.size() - 1];
				do{str >> skip;} while(skip != 'l'); // read to the end of the closing tag "</decimal>"
			}
			else // muls => end this
				break;
		}
		while(1) // Read Muls
		{
			do{str >> skip;} while(skip != 'n'); // Search the 'n' in "int" OR in "input" and check exactly which one is that
			str >> skip;
			if(skip == 't') // int => a member of "muls"
			{
				str >> skip; // Skip a ">"
				muls.push_back(0);
				str >> muls[muls.size() - 1];
				do{str >> skip;} while(skip != 't'); // read to the end of the closing tag "</int>"
			}
			else // Input => end this
				break;
		}
	}
	void fromJson(const std::string& in)
	{
		istringstream str(in);
		char skip;
		// Just in case you wanna run this in a loop; like in a server, maybe?
		sums.clear();
		muls.clear();
		do{str >> skip;} while(skip != 'K');
		str >> skip >> skip; // skip a "\":"
		str >> k; // First field
		do{str >> skip;} while(skip != '['); // Move to where the array begins
		do // Read Sums; this WILL fail if the array is empty, this scenario requires MANUAL numbers parsing too
		{
			sums.push_back(0);
			str >> sums[sums.size() - 1];
			str >> skip; // Read a "," OR a "]"
		} while(skip != ']');
		do{str >> skip;} while(skip != '['); // Move to where the array begins
		do // Read Muls; this WILL fail if the array is empty, this scenario requires MANUAL numbers parsing too
		{
			muls.push_back(0);
			str >> muls[muls.size() - 1];
			str >> skip; // Read a "," OR a "]"
		} while(skip != ']');
	}
};

struct Output
{
	double sumResult;
	int mulResult;
	vector<double> sorted;
	void solve(const Input& i)
	{
		sumResult = 0;
		for(double t: i.sums) sumResult += t;
		sumResult *= i.k;
		mulResult = 1;
		for(int t: i.muls) mulResult *= t;
		sorted = i.sums;
		sorted.reserve(i.sums.size() + i.muls.size());
		for(int t: i.muls) sorted.push_back(t);
		sort(sorted.begin(), sorted.end());
	}
	void toXml(std::string& out)
	{
		ostringstream str;
		// Set the flags so that 30.3 is written as 30.30
		ios::fmtflags oldflags = str.setf(ios::fixed, ios::floatfield);
		streamsize oldprec = str.precision(2);
		str << "<Output><SumResult>" << sumResult;
		// Reset flags to their default values
		str.precision(oldprec);
		str.flags(oldflags);
		str << "</SumResult><MulResult>" << mulResult << "</MulResult><SortedInputs>";
		for(double t: sorted)
		{
			str << "<decimal>" << t << "</decimal>";
		}
		str << "</SortedInputs></Output>";
		out = str.str();
	}
	void toJson(std::string& out)
	{
		ostringstream str;
		// Set the flags so that 30.3 is written as 30.30
		ios::fmtflags oldflags = str.setf(ios::fixed, ios::floatfield);
		streamsize oldprec = str.precision(2);
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
};

int main()
{
	Input input;
	Output output;
	string type, process;
	getline(cin, type);
	getline(cin, process);
	// NOTE: The task doesn't specify the CASE of the
	if(type == "Xml" || type == "xml" || type == "XML")
	{
		input.fromXml(process);
		output.solve(input);
		output.toXml(process);
	}
	else
	{
		input.fromJson(process);
		output.solve(input);
		output.toJson(process);
	}
	cout << process << "\n";
	return 0;
}
