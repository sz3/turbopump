/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "peer_client/IMessageSender.h"
#include "util/CallHistory.h"

class MockMessageSender : public IMessageSender
{
public:
	void digestPing(const Peer& peer, const TreeId& treeid, const MerklePoint& point);
	void digestPing(const Peer& peer, const TreeId& treeid, const std::deque<MerklePoint>& points);
	void requestKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last);
	void requestHealKey(const Peer& peer, const TreeId& treeid, unsigned long long key);

	void offerWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source);
	void demandWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source);
	void acknowledgeWrite(const Peer& peer, const std::string& filename, const std::string& version, unsigned long long size);

public:
	CallHistory _history;
};
