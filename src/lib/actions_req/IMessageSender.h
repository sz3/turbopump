#pragma once

#include "common/MerklePoint.h"
#include <deque>
class Peer;

class IMessageSender
{
public:
	virtual ~IMessageSender() {}

	virtual void merklePing(const Peer& peer, const MerklePoint& point) = 0;
	virtual void merklePing(const Peer& peer, const std::deque<MerklePoint>& points) = 0;
	virtual void requestKeyRange(const Peer& peer, unsigned long long first, unsigned long long last) = 0;
};
