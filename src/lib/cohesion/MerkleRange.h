#pragma once

#include "data_structures/merkle_location.h"

class MerkleRange
{
public:
	MerkleRange();
	MerkleRange(const merkle_location<unsigned long long>& location);

	void fromLocation(const merkle_location<unsigned long long>& location);

	unsigned long long first() const;
	unsigned long long last() const;

protected:
	unsigned long long _first;
	unsigned long long _last;
};

