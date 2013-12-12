#pragma once

#include "cohesion/IMerkleIndex.h"
#include "util/CallHistory.h"

class MockMerkleIndex : public IMerkleIndex
{
public:
	void add(const std::string& id);
	void remove(const std::string& id);

	MerklePoint top() const;
	std::deque<MerklePoint> diff(const MerklePoint& point) const;
	std::deque<std::string> enumerate(unsigned long long first, unsigned long long last, unsigned limit) const;

public:
	mutable CallHistory _history;

	MerklePoint _top;
	std::deque<MerklePoint> _diff;
	std::deque<std::string> _enumerate;
};
