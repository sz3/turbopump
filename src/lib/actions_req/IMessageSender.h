/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/MerklePoint.h"
#include <deque>
class Peer;
class TreeId;

class IMessageSender
{
public:
	virtual ~IMessageSender() {}

	virtual void merklePing(const Peer& peer, const TreeId& treeid, const MerklePoint& point) = 0;
	virtual void merklePing(const Peer& peer, const TreeId& treeid, const std::deque<MerklePoint>& points) = 0;
	virtual void requestKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last) = 0;
	virtual void dropKey(const Peer& peer, const std::string& filename) = 0;
};
