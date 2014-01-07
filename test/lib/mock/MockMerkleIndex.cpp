/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockMerkleIndex.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockMerkleIndex::add(const std::string& id)
{
	_history.call("add", id);
}

void MockMerkleIndex::remove(const std::string& id)
{
	_history.call("remove", id);
}

MerklePoint MockMerkleIndex::top() const
{
	_history.call("top");
	return _top;
}

std::deque<MerklePoint> MockMerkleIndex::diff(const MerklePoint& point) const
{
	_history.call("diff", MerklePointSerializer::toString(point));
	return _diff;
}

std::deque<std::string> MockMerkleIndex::enumerate(unsigned long long first, unsigned long long last, unsigned limit) const
{
	_history.call("enumerate", first, last);
	return _enumerate;
}
