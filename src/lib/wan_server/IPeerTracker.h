#pragma once

#include <memory>
class ConnectionWriteStream;
class Peer;
class PeerConnection;

// IActivePeers ?
class IPeerTracker
{
public:
	virtual ~IPeerTracker() {}
	virtual std::unique_ptr<ConnectionWriteStream> getWriter(const Peer& peer) = 0;
	virtual std::shared_ptr<PeerConnection> track(const Peer& peer) = 0;
};
