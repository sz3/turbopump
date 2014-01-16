/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IMerkleIndex.h"
#include "MerkleTree.h"
#include <map>
#include <vector>
class IHashRing;

class MerkleIndex : public IMerkleIndex
{
public:
	MerkleIndex(const IHashRing& ring);

	void add(const std::string& key);
	void remove(const std::string& key);

	const IMerkleTree& find(const std::string& id) const;
	const IMerkleTree& randomTree() const;

	std::vector<std::string> list() const; // for testing

protected:
	const IHashRing& _ring;
	std::map<std::string, MerkleTree> _forest;
	MerkleTree _emptyTree;
};
