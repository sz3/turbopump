/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "LocateKeys.h"

#include "IHashRing.h"
#include "membership/IMembership.h"
using std::string;
using std::vector;

LocateKeys::LocateKeys(const IHashRing& ring, const IMembership& membership)
	: _ring(ring)
	, _membership(membership)
{
}

vector<string> LocateKeys::locations(const string& name, unsigned mirrors) const
{
	return _ring.locations(name, mirrors);
}

bool LocateKeys::containsSelf(const vector<string>& locs) const
{
	return _membership.containsSelf(locs);
}

bool LocateKeys::keyIsMine(const string& name, unsigned mirrors) const
{
	vector<string> locs = locations(name, mirrors);
	if (locs.empty())
		return true; // special case
	return containsSelf(locs);
}
