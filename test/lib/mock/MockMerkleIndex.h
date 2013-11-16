#pragma once

#include "cohesion/IMerkleIndex.h"
#include "util/CallHistory.h"

class MockMerkleIndex : public IMerkleIndex
{
public:
	void add(const std::string& id);
	void remove(const std::string& id);

	std::deque<MerklePoint> diff(const MerklePoint& point) const;

public:
	mutable CallHistory _history;

	std::deque<MerklePoint> _diff;
};
