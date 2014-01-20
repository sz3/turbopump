/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "MockMerkleTree.h"
#include "cohesion/IMerkleIndex.h"
#include "util/CallHistory.h"

class MockMerkleIndex : public IMerkleIndex
{
public:
	void add(const std::string& key);
	void remove(const std::string& key);

	void splitTree(const std::string& where);
	void cannibalizeTree(const std::string& where);

	const IMerkleTree& find(const std::string& id) const;
	const IMerkleTree& randomTree() const;
	const IMerkleTree& unwantedTree() const;

public:
	MockMerkleTree _tree;
	mutable CallHistory _history;
};
