/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>
class IMerkleTree;

class IMerkleIndex
{
public:
	virtual ~IMerkleIndex() {}

	virtual void add(const std::string& key) = 0;
	virtual void remove(const std::string& key) = 0;

	virtual const IMerkleTree& find(const std::string& id) const = 0;
	virtual const IMerkleTree& randomTree() const = 0;
	virtual const IMerkleTree& unwantedTree() const = 0;

	// for membership/ring ranges, we'll need operations like this:
	// removeTree(id) -- take elements in id and put them in tree[id-1]
	// addTree(id) -- subdivide tree[id-1], putting everything after a given point into new tree[id]
};
