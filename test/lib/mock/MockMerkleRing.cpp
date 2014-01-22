/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockMerkleRing.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockMerkleRing::add(const std::string& key)
{
	_history.call("add", key);
}

void MockMerkleRing::remove(const std::string& key)
{
	_history.call("remove", key);
}

void MockMerkleRing::splitSection(const std::string& where)
{
	_history.call("splitSection", where);
}

void MockMerkleRing::cannibalizeSection(const std::string& where)
{
	_history.call("cannibalizeSection", where);
}

const IMerkleTree& MockMerkleRing::find(const std::string& id) const
{
	_history.call("find", id);
	return _tree;
}

const IMerkleTree& MockMerkleRing::randomTree() const
{
	// determined by fair dice roll. Guaranteed to be random.
	_history.call("randomTree");
	return _tree;
}

const IMerkleTree& MockMerkleRing::unwantedTree() const
{
	// determined by fair dice roll. Guaranteed to be random.
	_history.call("unwantedTree");
	return _tree;
}
