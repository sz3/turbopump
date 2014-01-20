/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MerkleIndex.h"

#include "common/MerklePoint.h"
#include "consistent_hashing/IHashRing.h"
#include "consistent_hashing/Hash.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"

#include "util/Random.h"
#include <algorithm>
using std::string;

MerkleIndex::MerkleIndex(const IHashRing& ring, const IMembership& membership)
	: _ring(ring)
	, _membership(membership)
{
}

// addFile
void MerkleIndex::add(const std::string& key)
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

	std::vector<string> locs;
	string section = _ring.lookup(key, locs, 3);
	MerkleTree& tree = _forest[section];
	initTree(tree, section, locs);
	tree.add(key);
}

void MerkleIndex::remove(const string& key)
{
	string section = _ring.section(key);
	std::map<string, MerkleTree>::iterator it = _forest.find(section);
	if (it == _forest.end())
		return;

	MerkleTree& tree = it->second;
	tree.remove(key);
	prune(it);
}

void MerkleIndex::initTree(MerkleTree& tree, const string& section, const std::vector<string>& locs)
{
	if (tree.empty())
	{
		tree.setId(section);
		if (std::find(locs.begin(), locs.end(), _membership.self()->uid) == locs.end())
			_unwanted.insert(section);
		else
			_wanted.insert(section);
	}
}

void MerkleIndex::prune(const std::map<string, MerkleTree>::iterator& it)
{
	MerkleTree& tree = it->second;
	if (tree.empty())
	{
		_unwanted.erase(tree.id());
		_wanted.erase(tree.id());
		_forest.erase(it);
	}
}

std::map<string, MerkleTree>::iterator MerkleIndex::prevTree(const std::map<string, MerkleTree>::iterator& it)
{
	std::map<string, MerkleTree>::iterator prev(it);
	if (prev == _forest.begin())
		prev = _forest.end();
	return --prev;
}

void MerkleIndex::splitTree(const string& where)
{
	std::vector<string> locs;
	string section = _ring.lookup(where, locs, 3);
	std::pair<std::map<string, MerkleTree>::iterator,bool> pear = _forest.emplace(std::make_pair(section, MerkleTree()));
	if (!pear.second)
		return;

	std::map<string, MerkleTree>::iterator prev = prevTree(pear.first);
	MerkleTree& sourceTree = prev->second;
	MerkleTree& newTree = pear.first->second;
	initTree(newTree, section, locs);

	auto fun = [&sourceTree, &newTree] (unsigned long long hash, const std::string& file) { newTree.add(file); sourceTree.remove(file); return true; };
	sourceTree.forEachInRange(fun, Hash::compute(where).integer(), ~0ULL);

	prune(prev);
	prune(pear.first);
}

void MerkleIndex::cannibalizeTree(const string& where)
{
	string section = _ring.section(where);
	std::map<string, MerkleTree>::iterator it = _forest.find(section);
	if (it == _forest.end())
		return;

	std::map<string, MerkleTree>::iterator prev = prevTree(it);
	MerkleTree& dyingTree = it->second;
	MerkleTree& refugeeTree = prev->second;

	auto fun = [&dyingTree, &refugeeTree] (unsigned long long hash, const std::string& file) { refugeeTree.add(file); dyingTree.remove(file); return true; };
	dyingTree.forEachInRange(fun, 0, ~0ULL);
	prune(it);
}

const IMerkleTree& MerkleIndex::find(const string& id) const
{
	std::map<string, MerkleTree>::const_iterator it = _forest.find(id);
	if (it == _forest.end())
		return _emptyTree;
	return it->second;
}

const IMerkleTree& MerkleIndex::randomTree() const
{
	if (_forest.empty())
		return _emptyTree;
	std::set<string>::const_iterator it = Random::select(_wanted.begin(), _wanted.end(), _wanted.size());
	if (it == _wanted.end())
		return _emptyTree;
	return find(*it);
}

const IMerkleTree& MerkleIndex::unwantedTree() const
{
	if (_forest.empty())
		return _emptyTree;
	std::set<string>::const_iterator it = Random::select(_unwanted.begin(), _unwanted.end(), _unwanted.size());
	if (it == _unwanted.end())
		return _emptyTree;
	return find(*it);
}

std::vector<string> MerkleIndex::list() const
{
	std::vector<string> treeIds;
	for (std::map<string, MerkleTree>::const_iterator it = _forest.begin(); it != _forest.end(); ++it)
		treeIds.push_back(it->first);
	return treeIds;
}
