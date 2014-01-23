/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MerkleIndex.h"

#include "MerkleTree.h"
#include "consistent_hashing/IHashRing.h"
#include "membership/IMembership.h"

#include "util/Random.h"
#include <algorithm>
#include <cassert>
using std::string;
using std::unique_ptr;

using map_type = std::map<unsigned char, unique_ptr<MerkleRing>>;

MerkleIndex::MerkleIndex(const IHashRing& ring, const IMembership& membership)
	: _ring(ring)
	, _membership(membership)
{
}

void MerkleIndex::add(const string& key, unsigned mirrors)
{
	// find appropriate merkle tree based on hash, mirrors

	unique_ptr<MerkleRing>& forest = _forest[mirrors];
	if (!forest)
		forest.reset( new MerkleRing(_ring, _membership, mirrors) );
	forest->add(key);
}

void MerkleIndex::remove(const string& key, unsigned mirrors)
{
	map_type::iterator it = _forest.find(mirrors);
	if (it == _forest.end())
		return;

	MerkleRing& forest = *it->second;
	forest.remove(key);
}

void MerkleIndex::splitSection(const string& where)
{
	for (map_type::iterator it = _forest.begin(); it != _forest.end(); ++it)
		it->second->splitSection(where);
}

void MerkleIndex::cannibalizeSection(const string& where)
{
	for (map_type::iterator it = _forest.begin(); it != _forest.end(); ++it)
		it->second->cannibalizeSection(where);
}

const IMerkleTree& MerkleIndex::find(const string& id, unsigned mirrors) const
{
	map_type::const_iterator it = _forest.find(mirrors);
	if (it == _forest.end())
		return MerkleTree::null();
	return it->second->find(id);
}

const IMerkleTree& MerkleIndex::randomTree() const
{
	if (_forest.empty())
		return MerkleTree::null();

	map_type::const_iterator start = _forest.begin();
	size_t elems = _forest.size();
	while (elems > 0 && start->first <= 1)
	{
		--elems;
		++start;
	}

	map_type::const_iterator it = Random::select(start, _forest.end(), elems);
	if (it == _forest.end())
		return MerkleTree::null();
	return it->second->randomTree();
}

const IMerkleTree& MerkleIndex::unwantedTree() const
{
	if (_forest.empty())
		return MerkleTree::null();

	map_type::const_iterator it = Random::select(_forest.begin(), _forest.end(), _forest.size());
	return it->second->unwantedTree();
}
