/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
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
