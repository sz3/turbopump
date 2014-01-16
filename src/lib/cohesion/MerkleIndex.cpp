/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MerkleIndex.h"

#include "common/MerklePoint.h"
#include "consistent_hashing/IHashRing.h"
#include "consistent_hashing/Hash.h"
#include "util/Random.h"
using std::string;

MerkleIndex::MerkleIndex(const IHashRing& ring)
	: _ring(ring)
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

	string section = _ring.section(key);
	MerkleTree& tree = _forest[section];
	if (tree.empty())
		tree.setId(section);
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
	if (tree.empty())
		_forest.erase(it);
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
	std::map<string, MerkleTree>::const_iterator it = Random::select(_forest.begin(), _forest.end(), _forest.size());
	return it->second;
}

std::vector<string> MerkleIndex::list() const
{
	std::vector<string> treeIds;
	for (std::map<string, MerkleTree>::const_iterator it = _forest.begin(); it != _forest.end(); ++it)
		treeIds.push_back(it->first);
	return treeIds;
}
