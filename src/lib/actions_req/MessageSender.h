/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IMessageSender.h"
class IPeerTracker;

class MessageSender : public IMessageSender
{
public:
	MessageSender(IPeerTracker& peers);

	void merklePing(const Peer& peer, const MerklePoint& point);
	void merklePing(const Peer& peer, const std::deque<MerklePoint>& points);
	void requestKeyRange(const Peer& peer, unsigned long long first, unsigned long long last);

protected:
	bool sendMessage(const Peer& peer, const std::string& message);

protected:
	IPeerTracker& _peers;
};
