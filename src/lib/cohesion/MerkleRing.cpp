/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MerkleRing.h"

#include "common/MerklePoint.h"
#include "consistent_hashing/IHashRing.h"
#include "consistent_hashing/Hash.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"

#include "util/Random.h"
#include <algorithm>
#include <endian.h>
using std::string;

MerkleRing::MerkleRing(const IHashRing& ring, const IMembership& membership, unsigned mirrors)
	: _ring(ring)
	, _membership(membership)
	, _mirrors(mirrors)
{
}

// addFile
void MerkleRing::add(const std::string& key)
{
	// find appropriate merkle tree based on hash, totalCopies

	/* maintain a list of trees.
	 * map?
	 * needs to reflect HashRing
	 * as hashring grows, index grows
	 * as hashring shrinks, index shrinks
	 * file hashes to hashRing section (first 64 bits?), and totalCopies parameter determines which merkle tree it addresses into.
	 *
	 * merkle sections contain the hashtoken that represents the primary mirror (e.g. "1:1", if it's 1's first hash range). This is true whether we're box 1, or box 2 (mirror 1), or box 3...
	 * merkle sections can be looked up by this hash token
	 **/

	string section = _ring.section(key);
	MerkleTree& tree = _forest[section];
	initTree(tree, section);
	tree.add(key);
}

void MerkleRing::remove(const string& key)
{
	string section = _ring.section(key);
	std::map<string, MerkleTree>::iterator it = _forest.find(section);
	if (it == _forest.end())
		return;

	MerkleTree& tree = it->second;
	tree.remove(key);
	prune(it);
}

void MerkleRing::initTree(MerkleTree& tree, const string& section)
{
	if (tree.empty())
	{
		tree.setId(TreeId(section, _mirrors));

		std::vector<string> locs = _ring.locationsFromHash(section, _mirrors);
		if (std::find(locs.begin(), locs.end(), _membership.self()->uid) == locs.end())
			_unwanted.insert(section);
		else
			_wanted.insert(section);
	}
}

void MerkleRing::prune(const std::map<string, MerkleTree>::iterator& it)
{
	MerkleTree& tree = it->second;
	if (tree.empty())
	{
		_unwanted.erase(tree.id().id);
		_wanted.erase(tree.id().id);
		_forest.erase(it);
	}
}

std::map<string, MerkleTree>::iterator MerkleRing::nextTree(const std::map<string, MerkleTree>::iterator& it)
{
	std::map<string, MerkleTree>::iterator next(it);
	if (++next == _forest.end())
		next = _forest.begin();
	return next;
}

void MerkleRing::splitSection(const string& where)
{
	if (_forest.empty())
		return;

	string section = _ring.section(where);
	std::pair<std::map<string, MerkleTree>::iterator,bool> pear = _forest.emplace(std::make_pair(section, MerkleTree()));
	if (!pear.second)
		return;

	std::map<string, MerkleTree>::iterator next = nextTree(pear.first);
	MerkleTree& sourceTree = next->second;
	MerkleTree& newTree = pear.first->second;
	initTree(newTree, section);

	// range we need to pull out of the sourceTree
	unsigned long long first = 0;
	unsigned long long last = Hash::compute(where).integer();

	// if sourceTree is the first tree and its id is < section, that means it remains the first node -- and that newTree is the last node.
	// So we need to grab its high-end keys instead of the usual low-end.
	if (next == _forest.begin())
	{
		if (sourceTree.id().id < section)
		{
			// would be nice to encapsulate this endianness nonsense somewhere.
			first = htobe64(Hash::fromBase64(sourceTree.id().id).integer());
			first = be64toh(++first);
		}
	}

	auto fun = [&sourceTree, &newTree] (unsigned long long hash, const std::string& file) { newTree.add(file); sourceTree.remove(file); return true; };
	sourceTree.forEachInRange(fun, first, last);

	prune(next);
	prune(pear.first);
}

void MerkleRing::cannibalizeSection(const string& where)
{
	string section = _ring.section(where);
	std::map<string, MerkleTree>::iterator it = _forest.find(section);
	if (it == _forest.end())
		return;

	std::map<string, MerkleTree>::iterator next = nextTree(it);
	MerkleTree& dyingTree = it->second;
	MerkleTree& refugeeTree = next->second;

	auto fun = [&dyingTree, &refugeeTree] (unsigned long long hash, const std::string& file) { refugeeTree.add(file); dyingTree.remove(file); return true; };
	dyingTree.forEachInRange(fun, 0, ~0ULL);
	prune(it);
}

const IMerkleTree& MerkleRing::find(const string& id) const
{
	std::map<string, MerkleTree>::const_iterator it = _forest.find(id);
	if (it == _forest.end())
		return MerkleTree::null();
	return it->second;
}

const IMerkleTree& MerkleRing::randomTree() const
{
	if (_forest.empty())
		return MerkleTree::null();
	std::set<string>::const_iterator it = Random::select(_wanted.begin(), _wanted.end(), _wanted.size());
	if (it == _wanted.end())
		return MerkleTree::null();
	return find(*it);
}

const IMerkleTree& MerkleRing::unwantedTree() const
{
	if (_forest.empty())
		return MerkleTree::null();
	std::set<string>::const_iterator it = Random::select(_unwanted.begin(), _unwanted.end(), _unwanted.size());
	if (it == _unwanted.end())
		return MerkleTree::null();
	return find(*it);
}

std::vector<string> MerkleRing::list() const
{
	std::vector<string> treeIds;
	for (std::map<string, MerkleTree>::const_iterator it = _forest.begin(); it != _forest.end(); ++it)
		treeIds.push_back(it->first);
	return treeIds;
}
