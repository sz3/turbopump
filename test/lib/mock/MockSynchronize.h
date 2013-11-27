#pragma once

#include "cohesion/ISynchronize.h"
#include "util/CallHistory.h"

class MockSynchronize : public ISynchronize
{
public:
	 void compare(const Peer& peer, const MerklePoint& point);
	 void pushKeyRange(const Peer& peer, unsigned long long first, unsigned long long last);

public:
	CallHistory _history;
};
