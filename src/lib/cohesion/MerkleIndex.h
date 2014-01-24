/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IMerkleIndex.h"
#include "MerkleRing.h"
#include <map>
#include <memory>
class IHashRing;
class IMembership;

// a forest of merkle trees.
// organized by section, and tracking ones we want to talk about with our peers "wanted"
// and those we want to offload ASAP ("unwanted")

class MerkleIndex : public IMerkleIndex
{
public:
	MerkleIndex(const IHashRing& ring, const IMembership& membership);

	void add(const std::string& key, unsigned mirrors=3);
	void remove(const std::string& key, unsigned mirrors=3);

	void splitSection(const std::string& where);
	void cannibalizeSection(const std::string& where);

	const IMerkleTree& find(const std::string& id, unsigned mirrors=3) const;
	const IMerkleTree& randomTree() const;
	const IMerkleTree& unwantedTree() const;

	void print() const; // for testing

protected:
	const IHashRing& _ring;
	const IMembership& _membership;
	std::map<unsigned char, std::unique_ptr<MerkleRing>> _forest;
};
