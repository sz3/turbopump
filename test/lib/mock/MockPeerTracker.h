#pragma once

#include "wan_server/IPeerTracker.h"
#include "util/CallHistory.h"
class IBufferedConnectionWriter;

class MockPeerTracker : public IPeerTracker
{
public:
	std::shared_ptr<IBufferedConnectionWriter> getWriter(const Peer& peer);
	std::shared_ptr<PeerConnection> track(const Peer& peer);

public:
	std::shared_ptr<IBufferedConnectionWriter> _writer;
	std::shared_ptr<PeerConnection> _conn;
	mutable CallHistory _history;
};
