/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/MerklePoint.h"
class Peer;

class ISynchronize
{
public:
	virtual ~ISynchronize() {}

	virtual void compare(const Peer& peer, const std::string& treeid, const MerklePoint& point) = 0;
	virtual void pushKeyRange(const Peer& peer, const std::string& treeid, unsigned long long first, unsigned long long last) = 0;
};
