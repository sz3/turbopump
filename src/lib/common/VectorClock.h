/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "vector_clock/bounded_time_vector.h"
#include <string>

class VectorClock : public vector_clock::bounded_time_vector<std::string,6>
{
public:
	VectorClock();
	VectorClock(const std::deque<VectorClock::clock>& clocks);

	VectorClock& increment(const std::string& key);
	VectorClock& increment(const std::string& key, uint64_t time);
	VectorClock& markDeleted();
	VectorClock& markDeleted(uint64_t time);
	bool isDeleted() const;
	bool isExpired(unsigned timeout) const;

	std::string toString() const;
	bool fromString(std::string str);
};
