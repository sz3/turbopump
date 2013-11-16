#pragma once

#include "MerklePoint.h"
#include <deque>
#include <string>

// need to account for the future with multiple merkle trees contained herein
class IMerkleIndex
{
public:
	virtual ~IMerkleIndex() {}

	virtual void add(const std::string& id) = 0;
	virtual void remove(const std::string& id) = 0;

	virtual std::deque<MerklePoint> diff(const MerklePoint& point) const = 0;
};
