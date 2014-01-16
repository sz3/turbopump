/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "cohesion/ISynchronize.h"
#include "util/CallHistory.h"

class MockSynchronize : public ISynchronize
{
public:
	 void compare(const Peer& peer, const std::string& treeid, const MerklePoint& point);
	 void pushKeyRange(const Peer& peer, const std::string& treeid, unsigned long long first, unsigned long long last);

public:
	CallHistory _history;
};
