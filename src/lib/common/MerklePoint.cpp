#include "MerklePoint.h"

#include <sstream>

std::string MerklePointSerializer::toString(const MerklePoint& point)
{
	std::stringstream ss;
	ss << point.location.key << " " << point.location.keybits << " " << point.hash;
	return ss.str();
}

bool MerklePointSerializer::fromString(MerklePoint& point, const std::string& str)
{
	std::istringstream iss(str);
	//iss >> std::noskipws;
	iss >> point.location.key;
	iss >> point.location.keybits;
	iss >> point.hash;
	return iss;
}
