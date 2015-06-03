/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "LocateKeys.h"

#include "Hash.h"
#include "IConsistentHashRing.h"
#include "membership/IKnowPeers.h"
#include "membership/Peer.h"
#include <algorithm>
using std::shared_ptr;
using std::string;
using std::vector;

LocateKeys::LocateKeys(const IConsistentHashRing& ring, const IKnowPeers& membership)
	: _ring(ring)
	, _membership(membership)
{
}

vector<string> LocateKeys::locations(const string& name, unsigned limit) const
{
	return _ring.locations(Hash(name), limit);
}

string LocateKeys::section(const string& name) const
{
	return _ring.nodeId(Hash(name));
}

bool LocateKeys::containsSelf(const vector<string>& locs) const
{
	shared_ptr<Peer> self = _membership.self();
	if (!self)
		return false;
	return std::find(locs.begin(), locs.end(), self->uid) != locs.end();
}

bool LocateKeys::keyIsMine(const string& name, unsigned limit) const
{
	vector<string> locs = locations(name, limit);
	if (locs.empty())
		return true; // special case
	return containsSelf(locs);
}

bool LocateKeys::sectionIsMine(const string& id, unsigned limit) const
{
	vector<string> locs = _ring.locations(Hash().fromHash(id), limit);
	if (locs.empty())
		return true; // special case
	return containsSelf(locs);
}
