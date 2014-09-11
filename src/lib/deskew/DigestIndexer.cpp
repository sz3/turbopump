/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DigestIndexer.h"

#include "common/MerklePoint.h"
#include "hashing/Hash.h"
#include "hashing/ILocateKeys.h"

#include "util/Random.h"
#include <algorithm>
#include <endian.h>
using std::string;

DigestIndexer::DigestIndexer(const ILocateKeys& locator, unsigned mirrors)
	: _locator(locator)
	, _mirrors(mirrors)
{
}

void DigestIndexer::update(const std::string& key, unsigned long long value)
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

	string section = _locator.section(key);

	DigestTree& tree = _forest[section];
	initTree(tree, section);
	tree.update(key, value);
}

void DigestIndexer::remove(const string& key)
{
	string section = _locator.section(key);

	std::map<string, DigestTree>::iterator it = _forest.find(section);
	if (it == _forest.end())
		return;

	DigestTree& tree = it->second;
	tree.remove(key);
	prune(it);
}

void DigestIndexer::initTree(DigestTree& tree, const string& section)
{
	if (tree.empty())
	{
		tree.setId(TreeId(section, _mirrors));

		if (_locator.sectionIsMine(section, _mirrors))
			_wanted.insert(section);
		else
			_unwanted.insert(section);
	}
}

void DigestIndexer::prune(const std::map<string, DigestTree>::iterator& it)
{
	DigestTree& tree = it->second;
	if (tree.empty())
	{
		_unwanted.erase(tree.id().id);
		_wanted.erase(tree.id().id);
		_forest.erase(it);
	}
}

std::map<string, DigestTree>::iterator DigestIndexer::nextTree(const std::map<string, DigestTree>::iterator& it)
{
	std::map<string, DigestTree>::iterator next(it);
	if (++next == _forest.end())
		next = _forest.begin();
	return next;
}

void DigestIndexer::splitSection(const string& where)
{
	if (_forest.empty())
		return;

	string section = _locator.section(where);
	std::pair<std::map<string, DigestTree>::iterator,bool> pear = _forest.emplace(std::make_pair(section, DigestTree()));
	if (!pear.second)
		return;

	std::map<string, DigestTree>::iterator next = nextTree(pear.first);
	DigestTree& sourceTree = next->second;
	DigestTree& newTree = pear.first->second;
	initTree(newTree, section);

	// range we need to pull out of the sourceTree
	unsigned long long first = 0;
	unsigned long long last = Hash(where).integer();

	// if sourceTree is the first tree and its id is < section, that means it remains the first node -- and that newTree is the last node.
	// So we need to grab its high-end keys instead of the usual low-end.
	if (next == _forest.begin())
	{
		if (sourceTree.id().id < section)
		{
			// would be nice to encapsulate this endianness nonsense somewhere.
			first = htobe64(Hash().fromBase64(sourceTree.id().id).integer());
			first = be64toh(++first);
		}
	}

	auto adder = [&newTree] (unsigned long long hash, const std::string& file) { newTree.add(file, hash); return true; };
	sourceTree.forEachInRange(adder, first, last);

	auto remover = [&sourceTree] (unsigned long long hash, const std::string& file) { sourceTree.remove(file); return true; };
	newTree.forEachInRange(remover, 0, ~0ULL);

	prune(next);
	prune(pear.first);
}

void DigestIndexer::cannibalizeSection(const string& where)
{
	string section = _locator.section(where);
	std::map<string, DigestTree>::iterator it = _forest.find(section);
	if (it == _forest.end())
		return;

	std::map<string, DigestTree>::iterator next = nextTree(it);
	if (next == it)
		return;
	DigestTree& dyingTree = it->second;
	DigestTree& refugeeTree = next->second;

	auto fun = [&refugeeTree] (unsigned long long hash, const std::string& file) { refugeeTree.add(file, hash); return true; };
	dyingTree.forEachInRange(fun, 0, ~0ULL);

	_unwanted.erase(dyingTree.id().id);
	_wanted.erase(dyingTree.id().id);
	_forest.erase(it);
}

const IDigestKeys& DigestIndexer::find(const string& id) const
{
	std::map<string, DigestTree>::const_iterator it = _forest.find(id);
	if (it == _forest.end())
		return DigestTree::null();
	return it->second;
}

const IDigestKeys& DigestIndexer::randomTree() const
{
	if (_forest.empty())
		return DigestTree::null();
	std::set<string>::const_iterator it = Random::select(_wanted.begin(), _wanted.end(), _wanted.size());
	if (it == _wanted.end())
		return DigestTree::null();
	return find(*it);
}

const IDigestKeys& DigestIndexer::unwantedTree() const
{
	if (_forest.empty())
		return DigestTree::null();
	std::set<string>::const_iterator it = Random::select(_unwanted.begin(), _unwanted.end(), _unwanted.size());
	if (it == _unwanted.end())
		return DigestTree::null();
	return find(*it);
}

std::vector<string> DigestIndexer::list() const
{
	std::vector<string> treeIds;
	for (std::map<string, DigestTree>::const_iterator it = _forest.begin(); it != _forest.end(); ++it)
		treeIds.push_back(it->first);
	return treeIds;
}
