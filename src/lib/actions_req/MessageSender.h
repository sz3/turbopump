/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IMessageSender.h"
class IPeerTracker;

class MessageSender : public IMessageSender
{
public:
	MessageSender(IPeerTracker& peers);

	// used by Synchronizer
	void digestPing(const Peer& peer, const TreeId& treeid, const MerklePoint& point);
	void digestPing(const Peer& peer, const TreeId& treeid, const std::deque<MerklePoint>& points);
	void requestKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last);\
	void requestHealKey(const Peer& peer, const TreeId& treeid, unsigned long long key);

	// used by actions, SkewCorrector, Callbacks, etc
	void offerWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source);
	void demandWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source);
	void dropKey(const Peer& peer, const std::string& filename);

protected:
	bool sendMessage(const Peer& peer, const std::string& message);

protected:
	IPeerTracker& _peers;
};
