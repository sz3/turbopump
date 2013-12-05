#pragma once

#include "wan_server/IPeerTracker.h"
#include "util/CallHistory.h"
class IIpSocket;

class MockPeerTracker : public IPeerTracker
{
public:
	std::unique_ptr<BufferedSocketWriter> getWriter(const Peer& peer) const;
	std::shared_ptr<PeerConnection> track(const Peer& peer);

public:
	std::shared_ptr<IIpSocket> _writerSocket;
	std::shared_ptr<PeerConnection> _conn;
	mutable CallHistory _history;
};
