#include "MockPeerTracker.h"

#include "membership/Peer.h"

std::shared_ptr<IBufferedConnectionWriter> MockPeerTracker::getWriter(const Peer& peer)
{
	_history.call("getWriter", peer.uid);
	return _writer;
}

std::shared_ptr<PeerConnection> MockPeerTracker::track(const Peer& peer)
{
	_history.call("track", peer.uid);
	return _conn;
}
