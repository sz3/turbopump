#include "MockPeerTracker.h"

#include "membership/Peer.h"
#include "wan_server/BufferedSocketWriter.h"

std::unique_ptr<BufferedSocketWriter> MockPeerTracker::getWriter(const Peer& peer) const
{
	_history.call("getWriter", peer.uid);
	return std::unique_ptr<BufferedSocketWriter>(new BufferedSocketWriter(_writerSocket));
}

std::shared_ptr<PeerConnection> MockPeerTracker::track(const Peer& peer)
{
	_history.call("track", peer.uid);
	return _conn;
}
