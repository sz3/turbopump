#pragma once

#include <memory>
class BufferedSocketWriter;
class Peer;
class PeerConnection;

// IActivePeers ?
class IPeerTracker
{
public:
	virtual ~IPeerTracker() {}
	virtual std::unique_ptr<BufferedSocketWriter> getWriter(const Peer& peer) const = 0;
	virtual std::shared_ptr<PeerConnection> track(const Peer& peer) = 0;
};
