#include "MockPeerTracker.h"

#include "membership/Peer.h"
#include "wan_server/ConnectionWriteStream.h"

std::unique_ptr<ConnectionWriteStream> MockPeerTracker::getWriter(const Peer& peer)
{
	_history.call("getWriter", peer.uid);
	return std::unique_ptr<ConnectionWriteStream>(new ConnectionWriteStream(_writer, peer.nextActionId()));
}

std::shared_ptr<PeerConnection> MockPeerTracker::track(const Peer& peer)
{
	_history.call("track", peer.uid);
	return _conn;
}
