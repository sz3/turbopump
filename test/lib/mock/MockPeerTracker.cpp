/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
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
