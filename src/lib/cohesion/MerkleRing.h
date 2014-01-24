/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IMerkleRing.h"
#include "MerkleTree.h"
#include <map>
#include <set>
#include <vector>
class IHashRing;
class IMembership;

// a ring of merkle trees.
// organized by section, and tracking ones we want to talk about with our peers "wanted"
// and those we want to offload ASAP ("unwanted")

class MerkleRing : public IMerkleRing
{
public:
	MerkleRing(const IHashRing& ring, const IMembership& membership, unsigned mirrors=3);

	void add(const std::string& key);
	void remove(const std::string& key);

	void splitSection(const std::string& where);
	void cannibalizeSection(const std::string& where);

	const IMerkleTree& find(const std::string& id) const;
	const IMerkleTree& randomTree() const;
	const IMerkleTree& unwantedTree() const;

	std::vector<std::string> list() const; // for testing

private:
	void initTree(MerkleTree& tree, const std::string& section);
	void prune(const std::map<std::string, MerkleTree>::iterator& it);
	std::map<std::string, MerkleTree>::iterator nextTree(const std::map<std::string, MerkleTree>::iterator& it);

protected:
	const IHashRing& _ring;
	const IMembership& _membership;
	unsigned _mirrors;

	std::map<std::string, MerkleTree> _forest;
	std::set<std::string> _wanted;
	std::set<std::string> _unwanted;
};
