#pragma once

#include "cohesion/ICorrectSkew.h"
#include "util/CallHistory.h"

class MockSkewCorrector : public ICorrectSkew
{
public:
	void healKey(const Peer& peer, unsigned long long key);
	void pushKeyRange(const Peer& peer, unsigned long long first, unsigned long long last);

public:
	CallHistory _history;
};
