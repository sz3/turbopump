#include "MockPeerTracker.h"

#include "membership/Peer.h"

std::shared_ptr<PeerConnection> MockPeerTracker::track(const Peer& peer)
{
	_history.call("track", peer.uid);
	return _conn;
}

bool MockPeerTracker::decode(const Peer& peer, const std::string& encoded, std::shared_ptr<PeerConnection>& conn, std::string& decoded)
{
	_history.call("decode", peer.uid, encoded);
	if (!_conn)
		return false;

	conn = _conn;
	decoded = encoded;
	return true;
}
