/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MirrorToPeer.h"

#include "actions_req/IMessageSender.h"
#include "actions_req/WriteActionSender.h"
#include "common/KeyMetadata.h"
#include "consistent_hashing/IHashRing.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include <algorithm>
#include <iostream>
#include <memory>
using std::shared_ptr;

MirrorToPeer::MirrorToPeer(const IHashRing& ring, const IMembership& membership, IMessageSender& messenger, IPeerTracker& peers)
	: _ring(ring)
	, _membership(membership)
	, _messenger(messenger)
	, _peers(peers)
{
}

// need to tell me what mirror number I'm supposed to be. e.g. 0, 1, 2, 3. Also need number of desired copies. (0 == infinite?)
// TODO: specify number of copies -- 1,2,3,5
// TODO: specify consistency?
bool MirrorToPeer::run(KeyMetadata md, IDataStoreReader::ptr contents)
{
	std::vector<std::string> locations = _ring.locations(md.filename, md.totalCopies);
	shared_ptr<Peer> self = _membership.self();
	shared_ptr<Peer> peer;

	// TODO: split this command into two? One for a local write, one for a wan one...
	// if I'm the local write and shouldn't have the file long-term, update the md to say so
	if (md.mirror == 0)
	{
		if (std::find(locations.begin(), locations.end(), self->uid) == locations.end())
			md.source = self->uid;
	}

	unsigned next = md.mirror;
	if (next >= md.totalCopies)
	{
		if (!md.source.empty())
		{
			shared_ptr<Peer> extraMirror = _membership.lookup(md.source);
			if (extraMirror)
				_messenger.dropKey(*extraMirror, md.filename);
		}
		return false;
	}

	for (; next < locations.size(); ++next)
	{
		peer = _membership.lookup(locations[next]);
		if (peer != self)
			break;
	}
	if (!peer || peer == self)
		return false;

	md.mirror = next+1;
	WriteActionSender client(_peers);
	return client.store(*peer, md, contents);
}
