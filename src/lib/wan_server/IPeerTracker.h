#pragma once

#include <memory>

class Peer;
class PeerConnection;

// IActivePeers ?
class IPeerTracker
{
public:
	virtual ~IPeerTracker() {}
	virtual std::shared_ptr<PeerConnection> track(const Peer& peer) = 0;
};
