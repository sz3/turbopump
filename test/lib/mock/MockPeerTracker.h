#pragma once

#include "wan_server/IPeerTracker.h"
#include "util/CallHistory.h"
class BufferedConnectionWriter;

class MockPeerTracker : public IPeerTracker
{
public:
	std::unique_ptr<ConnectionWriteStream> getWriter(const Peer& peer);
	std::shared_ptr<PeerConnection> track(const Peer& peer);

public:
	std::shared_ptr<BufferedConnectionWriter> _writer;
	std::shared_ptr<PeerConnection> _conn;
	mutable CallHistory _history;
};
