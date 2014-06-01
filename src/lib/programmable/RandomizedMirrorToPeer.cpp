/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "RandomizedMirrorToPeer.h"

#include "actions/WriteParams.h"
#include "actions_req/WriteActionSender.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include <memory>
using std::shared_ptr;

RandomizedMirrorToPeer::RandomizedMirrorToPeer(const IMembership& membership, IPeerTracker& peers)
	: _membership(membership)
	, _peers(peers)
{
}

bool RandomizedMirrorToPeer::run(WriteParams params, IDataStoreReader::ptr contents)
{
	shared_ptr<Peer> peer = _membership.randomPeer();
	if (!peer)
		return false;

	if (params.mirror >= params.totalCopies)
		return false;

	WriteActionSender client(_peers, true);
	return client.store(*peer, params, contents);
}
