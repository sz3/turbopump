#pragma once

#include "cohesion/IMessageSender.h"
#include "util/CallHistory.h"

class MockMessageSender : public IMessageSender
{
public:
	void merklePing(const Peer& peer, const MerklePoint& point);
	void merklePing(const Peer& peer, const std::deque<MerklePoint>& points);
	void requestKeyRange(const Peer& peer, unsigned long long first, unsigned long long last);

public:
	CallHistory _history;
};
