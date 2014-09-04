/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/MerklePoint.h"
class Peer;
class TreeId;

class ISynchronize
{
public:
	virtual ~ISynchronize() {}

	virtual void compare(const Peer& peer, const TreeId& treeid, const MerklePoint& point, bool isSyncResponse=false) = 0;
};
