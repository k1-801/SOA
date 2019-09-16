#include "Input.hpp"

void Input::fromJson(const QString& in)
{
	std::istringstream str(in);
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
