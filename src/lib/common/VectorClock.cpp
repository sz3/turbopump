/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "VectorClock.h"

#include <algorithm>
#include <deque>
#include <sstream>
using std::deque;

VectorClock::VectorClock()
{
}

VectorClock::VectorClock(const std::deque<VectorClock::clock>& clocks)
	: bounded_version_vector<std::string,10>(clocks)
{
}

namespace {
	std::ostream& operator<<(std::ostream& outstream, const VectorClock::clock& clock)
	{
		outstream << clock.key << ":" << clock.count;
		return outstream;
	}

	std::ostream& operator<<(std::ostream& outstream, const deque<VectorClock::clock>& clocks)
	{
		outstream << clocks.size();
		for (auto it = clocks.begin(); it != clocks.end(); ++it)
			outstream << "," << *it;
		return outstream;
	}

	std::istream& operator>>(std::istream& instream, VectorClock::clock& clock)
	{
		std::string clockStr;
		instream >> clockStr;

		std::replace(clockStr.begin(), clockStr.end(), ':', ' ');
		std::istringstream iss(clockStr);
		iss >> clock.key;
		iss >> clock.count;

		return instream;
	}

	std::istream& operator>>(std::istream& instream, deque<VectorClock::clock>& clocks)
	{
		unsigned size = 0;
		instream >> size;
		for (unsigned i = 0; i < size; ++i)
		{
			VectorClock::clock cl;
			instream >> cl;
			clocks.push_back(cl);
		}
		return instream;
	}
}

std::string VectorClock::toString() const
{
	std::stringstream ss;
	ss << _clocks;
	return ss.str();
}

bool VectorClock::fromString(std::string str)
{
	std::replace(str.begin(), str.end(), ',', ' ');

	std::deque<VectorClock::clock> clocks;
	std::istringstream iss(str);
	iss >> clocks;

	if (!iss)
		return false;
	_clocks = clocks;
	return true;
}
