/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MirrorToPeer.h"

#include "actions/WriteParams.h"
#include "actions_req/WriteActionSender.h"
#include "consistent_hashing/IHashRing.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include <algorithm>
#include <iostream>
#include <memory>
using std::shared_ptr;

MirrorToPeer::MirrorToPeer(const IHashRing& ring, const IMembership& membership, IPeerTracker& peers, bool blocking)
	: _ring(ring)
	, _membership(membership)
	, _peers(peers)
	, _blocking(blocking)
{
}

// need to tell me what mirror number I'm supposed to be. e.g. 0, 1, 2, 3. Also need number of desired copies. (0 == infinite?)
// TODO: specify number of copies -- 1,2,3,5
// TODO: specify consistency?
bool MirrorToPeer::run(WriteParams& params, IDataStoreReader::ptr contents)
{
	WriteActionSender client(_peers, _blocking);
	if (!params.outstream)
	{
		shared_ptr<Peer> peer;
		if (!chooseMirror(params, peer))
			return false;
		params.outstream = client.open(*peer, params);
		if (!params.outstream)
			return false;
	}

	return client.store(*params.outstream, params, contents);
}

bool MirrorToPeer::chooseMirror(WriteParams& params, std::shared_ptr<Peer>& peer)
{
	std::vector<std::string> locations = _ring.locations(params.filename, params.totalCopies);
	shared_ptr<Peer> self = _membership.self();
	if (!self)
		return false;

	// the first write is the only one that might be out of order.
	// also, it might be that the first guy (source) needs to drop the file once all copies are written.
	if (params.mirror == 0)
		params.source = self->uid;

	unsigned next = params.mirror;
	if (next >= params.totalCopies)
		return false;

	for (; next < locations.size(); ++next)
	{
		if (locations[next] == params.source)
			continue;
		peer = _membership.lookup(locations[next]);
		if (peer != self)
			break;
	}
	if (!peer || peer == self || peer->uid == params.source)
		return false;

	params.mirror = next+1;
	return true;
}
