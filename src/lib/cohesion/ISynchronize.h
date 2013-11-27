#pragma once

#include "common/MerklePoint.h"
class Peer;

class ISynchronize
{
public:
	virtual ~ISynchronize() {}

	virtual void compare(const Peer& peer, const MerklePoint& point) = 0;
	virtual void pushKeyRange(const Peer& peer, unsigned long long first, unsigned long long last) = 0;
};
