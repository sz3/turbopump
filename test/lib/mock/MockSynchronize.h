#pragma once

#include "cohesion/ISynchronize.h"
#include "util/CallHistory.h"

class MockSynchronize : public ISynchronize
{
public:
	 void compare(const Peer& peer, const MerklePoint& point);

public:
	CallHistory _history;
};
