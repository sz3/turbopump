#pragma once

#include "common/MerklePoint.h"
class Peer;

class ISynchronize
{
public:
	virtual ~ISynchronize() {}

	virtual void compare(const Peer& peer, const MerklePoint& point) = 0;
};
