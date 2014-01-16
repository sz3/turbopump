/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockMerkleTree.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

MockMerkleTree::MockMerkleTree()
	: _empty(false)
	, _id("tim")
{
}

std::string MockMerkleTree::id() const
{
	return _id;
}

void MockMerkleTree::add(const std::string& key)
{
	_history.call("add", key);
}

void MockMerkleTree::remove(const std::string& key)
{
	_history.call("remove", key);
}

bool MockMerkleTree::empty() const
{
	_history.call("empty");
	return _empty;
}

MerklePoint MockMerkleTree::top() const
{
	_history.call("top");
	return _top;
}

std::deque<MerklePoint> MockMerkleTree::diff(const MerklePoint& point) const
{
	_history.call("diff", MerklePointSerializer::toString(point));
	return _diff;
}

std::deque<std::string> MockMerkleTree::enumerate(unsigned long long first, unsigned long long last, unsigned limit) const
{
	_history.call("enumerate", first, last);
	return _enumerate;
}
