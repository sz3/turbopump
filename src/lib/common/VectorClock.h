/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_structures/bounded_version_vector.h"

class VectorClock : public turbo::bounded_version_vector<std::string,10>
{
public:
	VectorClock();
	VectorClock(const std::deque<VectorClock::clock>& clocks);

	bool isDeleted() const;
	void markDeleted();

	std::string toString() const;
	bool fromString(std::string str);
};
