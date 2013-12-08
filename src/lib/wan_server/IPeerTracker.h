#pragma once

#include <memory>
class IBufferedConnectionWriter;
class Peer;
class PeerConnection;

// IActivePeers ?
class IPeerTracker
{
public:
	virtual ~IPeerTracker() {}
	virtual std::shared_ptr<IBufferedConnectionWriter> getWriter(const Peer& peer) = 0;
	virtual std::shared_ptr<PeerConnection> track(const Peer& peer) = 0;
};
