/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "KeyTabulator.h"

#include "DigestIndexer.h"
#include "DigestTree.h"
#include "consistent_hashing/IHashRing.h"
#include "membership/IMembership.h"

#include "serialize/StringUtil.h"
#include "util/Random.h"
#include <algorithm>
#include <cassert>
#include <iostream>
using std::string;
using std::unique_ptr;

using map_type = std::map<unsigned char, unique_ptr<IDigestIndexer>>;

KeyTabulator::KeyTabulator(const IHashRing& ring, const IMembership& membership)
	: _ring(ring)
	, _membership(membership)
{
}

void KeyTabulator::add(const string& key, unsigned mirrors)
{
	// find appropriate merkle tree based on hash, mirrors

	unique_ptr<IDigestIndexer>& forest = _forest[mirrors];
	if (!forest)
		forest.reset( new DigestIndexer(_ring, _membership, mirrors) );
	forest->add(key);
}

void KeyTabulator::remove(const string& key, unsigned mirrors)
{
	map_type::iterator it = _forest.find(mirrors);
	if (it == _forest.end())
		return;

	IDigestIndexer& forest = *it->second;
	forest.remove(key);
}

void KeyTabulator::splitSection(const string& where)
{
	for (map_type::iterator it = _forest.begin(); it != _forest.end(); ++it)
		it->second->splitSection(where);
}

void KeyTabulator::cannibalizeSection(const string& where)
{
	for (map_type::iterator it = _forest.begin(); it != _forest.end(); ++it)
		it->second->cannibalizeSection(where);
}

const IDigestKeys& KeyTabulator::find(const string& id, unsigned mirrors) const
{
	map_type::const_iterator it = _forest.find(mirrors);
	if (it == _forest.end())
		return DigestTree::null();
	return it->second->find(id);
}

const IDigestKeys& KeyTabulator::randomTree() const
{
	if (_forest.empty())
		return DigestTree::null();

	map_type::const_iterator it = Random::select(_forest.begin(), _forest.end(), _forest.size());
	if (it->first == 1)
	{
		if (it == _forest.begin())
			++it;
		else
			--it;
	}
	if (it == _forest.end())
		return DigestTree::null();

	return it->second->randomTree();
}

const IDigestKeys& KeyTabulator::unwantedTree() const
{
	if (_forest.empty())
		return DigestTree::null();

	map_type::const_iterator start = _forest.begin();
	size_t elems = _forest.size();
	if (start->first == 0)
	{
		++start;
		--elems;
	}
	map_type::const_iterator it = Random::select(_forest.begin(), _forest.end(), elems);
	return it->second->unwantedTree();
}

// testing
void KeyTabulator::print() const
{
	//for (map_type::const_iterator it = _forest.begin(); it != _forest.end(); ++it)
	//	std::cout << StringUtil::str(it->first) << " : " << StringUtil::join(it->second->list()) << std::endl;
}
