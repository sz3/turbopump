/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MirrorToPeer.h"

#include "api/WriteInstructions.h"
#include "hashing/ILocateKeys.h"
#include "membership/IKnowPeers.h"
#include "membership/Peer.h"

#include "serialize/format.h"
#include <iostream>
using std::shared_ptr;
using std::string;

MirrorToPeer::MirrorToPeer(const ILocateKeys& locator, const IKnowPeers& membership)
	: _locator(locator)
	, _membership(membership)
{
}

bool MirrorToPeer::chooseMirror(WriteInstructions& params, std::shared_ptr<Peer>& peer)
{
	unsigned next = params.mirror;
	if (next >= params.copies)
		return false;

	std::vector<string> locations = _locator.locations(params.name, params.copies);
	shared_ptr<Peer> self = _membership.self();
	if (!self)
		return false;

	// the first write is the only one that might be out of order.
	// also, it might be that the first guy (source) needs to drop the file once all copies are written.
	// TODO: fix stupid loop bug (wasted effort) when the last location is the source...
	if (params.mirror == 0)
		params.source = self->uid;

	for (; next < locations.size(); ++next)
	{
		const string& loc = locations[next];
		if (loc == params.source || loc == self->uid)
			continue;
		peer = _membership.lookup(loc);
		break;
	}
	if (!peer || peer == self || peer->uid == params.source)
		return false;

	std::cerr << fmt::format("logger: MirrorToPeer choosing {} for file {}:{}", peer->uid, params.name, params.version) << std::endl;
	params.mirror = next+1;
	return true;
}
