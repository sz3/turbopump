/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MirrorToPeer.h"

#include "actions/WriteParams.h"
#include "hashing/ILocateKeys.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
using std::shared_ptr;

MirrorToPeer::MirrorToPeer(const ILocateKeys& locator, const IMembership& membership)
	: _locator(locator)
	, _membership(membership)
{
}

bool MirrorToPeer::chooseMirror(WriteParams& params, std::shared_ptr<Peer>& peer)
{
	unsigned next = params.mirror;
	if (next >= params.totalCopies)
		return false;

	std::vector<std::string> locations = _locator.locations(params.filename, params.totalCopies);
	shared_ptr<Peer> self = _membership.self();
	if (!self)
		return false;

	// the first write is the only one that might be out of order.
	// also, it might be that the first guy (source) needs to drop the file once all copies are written.
	if (params.mirror == 0)
		params.source = self->uid;

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
