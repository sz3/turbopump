#pragma once

#include "wan_server/IPeerTracker.h"
#include "util/CallHistory.h"

class MockPeerTracker : public IPeerTracker
{
public:
	std::shared_ptr<PeerConnection> track(const Peer& peer);
	bool decode(const Peer& peer, const std::string& encoded, std::shared_ptr<PeerConnection>& conn, std::string& decoded);

public:
	std::shared_ptr<PeerConnection> _conn;
	CallHistory _history;
};
