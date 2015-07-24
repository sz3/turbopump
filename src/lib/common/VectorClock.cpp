/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "VectorClock.h"

#include "WallClock.h"
#include "serialize/base64.h"
#include "serialize/str.h"

#include <algorithm>
#include <deque>
#include <sstream>
using std::deque;
using namespace turbo;

using base_class = vector_clock::bounded_time_vector<std::string,10>;

VectorClock::VectorClock()
	: base_class()
{
}

VectorClock::VectorClock(const std::deque<VectorClock::clock>& clocks)
	: base_class(clocks)
{
}

namespace {
	std::ostream& operator<<(std::ostream& outstream, const VectorClock::clock& clock)
	{
		outstream << clock.key << "." << base64::encode_bin(clock.time);
		if (clock.count != 0)
			outstream << "." << clock.count;
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
		if (!instream)
			return instream;

		clock.time = 0;
		clock.count = 0;

		size_t pos = clockStr.find('.');
		if (pos != std::string::npos && (pos+1) != std::string::npos)
		{
			clock.key = clockStr.substr(0, pos);
			size_t count_pos = clockStr.find('.', pos+1);

			clock.time = base64::decode_as<uint64_t>(clockStr.substr(pos+1, count_pos));
			if (count_pos != std::string::npos && (count_pos+1) != std::string::npos)
				clock.count = std::stoul(clockStr.substr(count_pos+1));
		}
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
			if (!instream)
				break;
			clocks.push_back(cl);
		}
		return instream;
	}
}

VectorClock& VectorClock::increment(const std::string& key)
{
	base_class::increment(key, WallClock::now());
	return *this;
}

VectorClock& VectorClock::markDeleted()
{
	increment("delete");
	return *this;
}

bool VectorClock::isDeleted() const
{
	if (empty())
		return false;
	return _clocks.front().key == "delete";
}

bool VectorClock::isExpired(unsigned timeout) const
{
	if ( !isDeleted() )
		return false;
	uint64_t expiry = _clocks.front().time + timeout;
	uint64_t now = WallClock::now();
	return now >= expiry;
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
