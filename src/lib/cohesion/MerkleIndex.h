/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IMerkleIndex.h"
#include "MerkleTree.h"
#include <map>
#include <set>
#include <vector>
class IHashRing;
class IMembership;

class MerkleIndex : public IMerkleIndex
{
public:
	MerkleIndex(const IHashRing& ring, const IMembership& membership);

	void add(const std::string& key);
	void remove(const std::string& key);

	void splitTree(const std::string& where);
	void cannibalizeTree(const std::string& where);

	const IMerkleTree& find(const std::string& id) const;
	const IMerkleTree& randomTree() const;
	const IMerkleTree& unwantedTree() const;

	std::vector<std::string> list() const; // for testing

private:
	void initTree(MerkleTree& tree, const std::string& section, const std::vector<std::string>& locs);
	void prune(const std::map<std::string, MerkleTree>::iterator& it);
	std::map<std::string, MerkleTree>::iterator prevTree(const std::map<std::string, MerkleTree>::iterator& it);

protected:
	const IHashRing& _ring;
	const IMembership& _membership;
	std::map<std::string, MerkleTree> _forest;
	std::set<std::string> _wanted;
	std::set<std::string> _unwanted;
	MerkleTree _emptyTree;
};
