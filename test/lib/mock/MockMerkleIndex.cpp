/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockMerkleIndex.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockMerkleIndex::add(const std::string& key)
{
	_history.call("add", key);
}

void MockMerkleIndex::remove(const std::string& key)
{
	_history.call("remove", key);
}

const IMerkleTree& MockMerkleIndex::find(const std::string& id) const
{
	_history.call("find", id);
	return _tree;
}

const IMerkleTree& MockMerkleIndex::randomTree() const
{
	// determined by fair dice roll. Guaranteed to be random.
	_history.call("randomTree");
	return _tree;
}
