/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
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

	// getWriter returns a shared_ptr only because it makes it easier to test. Alas.
	virtual std::shared_ptr<IBufferedConnectionWriter> getWriter(const Peer& peer) = 0;
	virtual std::shared_ptr<PeerConnection> track(const Peer& peer) = 0;
};
