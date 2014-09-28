/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IStatusReporter.h"

#include "IProcessState.h"
#include "hashing/IConsistentHashRing.h"
#include "membership/IMembership.h"

class StatusReporter : public IStatusReporter
{
public:
	StatusReporter(const IConsistentHashRing& ring, const IMembership& membership, const IProcessState& state)
		: _ring(ring)
		, _membership(membership)
		, _state(state)
	{}

	std::string status(const std::string& system) const
	{
		if (system == "membership")
			return _membership.toString();
		else if (system == "ring")
			return _ring.toString();
		else
			return _state.summary();
	}

protected:
	const IConsistentHashRing& _ring;
	const IMembership& _membership;
	const IProcessState& _state;
};
