/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "LocateKeys.h"

#include "Hash.h"
#include "IConsistentHashRing.h"
#include "membership/IMembership.h"
using std::string;
using std::vector;

LocateKeys::LocateKeys(const IConsistentHashRing& ring, const IMembership& membership)
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

// why bother implementing this in membership, given that it's only used here? Just call MyMemberId() from here...
// in fact, maybe some of the more "locate-y" calls in the hash ring ought to be in here as well. Just have give
bool LocateKeys::containsSelf(const vector<string>& locs) const
{
	return _membership.containsSelf(locs);
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
	vector<string> locs = _ring.locations(Hash().fromBase64(id), limit);
	if (locs.empty())
		return true; // special case
	return containsSelf(locs);
}
