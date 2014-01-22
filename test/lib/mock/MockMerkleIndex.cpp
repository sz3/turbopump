/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockMerkleIndex.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockMerkleIndex::add(const std::string& key, unsigned mirrors)
{
	_history.call("add", key, mirrors);
}

void MockMerkleIndex::remove(const std::string& key, unsigned mirrors)
{
	_history.call("remove", key, mirrors);
}

void MockMerkleIndex::splitSection(const std::string& where)
{
	_history.call("splitSection", where);
}

void MockMerkleIndex::cannibalizeSection(const std::string& where)
{
	_history.call("cannibalizeSection", where);
}

const IMerkleTree& MockMerkleIndex::find(const std::string& id, unsigned mirrors) const
{
	_history.call("find", id, mirrors);
	return _tree;
}

const IMerkleTree& MockMerkleIndex::randomTree() const
{
	// determined by fair dice roll. Guaranteed to be random.
	_history.call("randomTree");
	return _tree;
}

const IMerkleTree& MockMerkleIndex::unwantedTree() const
{
	// determined by fair dice roll. Guaranteed to be random.
	_history.call("unwantedTree");
	return _tree;
}
