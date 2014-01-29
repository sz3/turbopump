/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockDigestTree.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

MockDigestTree::MockDigestTree()
	: _empty(false)
	, _id("tim", 3)
{
}

const TreeId& MockDigestTree::id() const
{
	return _id;
}

void MockDigestTree::add(const std::string& key)
{
	_history.call("add", key);
}

void MockDigestTree::remove(const std::string& key)
{
	_history.call("remove", key);
}

bool MockDigestTree::empty() const
{
	_history.call("empty");
	return _empty;
}

MerklePoint MockDigestTree::top() const
{
	_history.call("top");
	return _top;
}

std::deque<MerklePoint> MockDigestTree::diff(const MerklePoint& point) const
{
	_history.call("diff", MerklePointSerializer::toString(point));
	return _diff;
}

std::deque<std::string> MockDigestTree::enumerate(unsigned long long first, unsigned long long last, unsigned limit) const
{
	_history.call("enumerate", first, last);
	return _enumerate;
}
