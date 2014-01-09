/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MirrorToPeer.h"

#include "actions_req/WriteActionSender.h"
#include "common/KeyMetadata.h"
#include "consistent_hashing/IHashRing.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include <iostream>
#include <memory>
using std::shared_ptr;

MirrorToPeer::MirrorToPeer(const IHashRing& ring, const IMembership& membership, IPeerTracker& peers)
	: _ring(ring)
	, _membership(membership)
	, _peers(peers)
{
}

// need to tell me what mirror number I'm supposed to be. e.g. 0, 1, 2, 3. Also need number of desired copies. (0 == infinite?)
// TODO: specify number of copies -- 1,2,3,5
// TODO: specify consistency?
bool MirrorToPeer::run(KeyMetadata md, IDataStoreReader::ptr contents)
{
	std::vector<std::string> locations = _ring.lookup(md.filename, md.totalCopies);
	shared_ptr<Peer> self = _membership.self();
	shared_ptr<Peer> peer;

	unsigned next = md.mirror;
	for (; next < locations.size(); ++next)
	{
		peer = _membership.lookup(locations[next]);
		if (peer != self)
			break;
	}
	if (!peer || peer == self)
		return false;

	WriteActionSender client(_peers);
	return client.store(*peer, md.filename, contents);
}
