/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DigestTree.h"

#include "common/MerklePoint.h"
#include "hashing/Hash.h"
using namespace std::placeholders;

const DigestTree& DigestTree::null()
{
	static DigestTree tree;
	return tree;
}

void DigestTree::setId(const TreeId& id)
{
	_id = id;
}

const TreeId& DigestTree::id() const
{
	return _id;
}

void DigestTree::update(const std::string& key, unsigned long long value)
{
	// TODO: update merkle_tree::insert (or create alternate method) to allow updates to existing values!
	// also, just improve the merkle_tree api in general... it works, now make the code less horrifying.

	unsigned long long keyhash = Hash(key).integer();
	merkle_tree<unsigned long long, unsigned long long, std::string>::pair* ptr = _tree.find(keyhash);
	if (ptr != NULL)
	{
		value ^= std::get<0>(ptr->second);
		_tree.remove(keyhash);
	}
	else
		value ^= keyhash;
	_tree.insert(keyhash, value, key);
}

void DigestTree::add(const std::string& key, unsigned long long hash)
{
	// not part of the public interface. For places we need to load a hash we already have.
	// (split section)
	unsigned long long keyhash = Hash(key).integer();
	_tree.insert(keyhash, hash, key);
}

void DigestTree::remove(const std::string& key)
{
	unsigned long long keyhash = Hash(key).integer();
	_tree.remove(keyhash);
}

bool DigestTree::empty() const
{
	return _tree.top() == MerklePoint::null();
}

MerklePoint DigestTree::top() const
{
	return _tree.top();
}

std::deque<MerklePoint> DigestTree::diff(const MerklePoint& point) const
{
	return _tree.diff(point.location, point.hash);
}

// TODO: unsigned long long& start?
//       return "next"? ???
std::deque<std::string> DigestTree::enumerate(unsigned long long first, unsigned long long last, unsigned limit) const
{
	std::deque<std::string> files;
	auto fun = [&,limit] (unsigned long long hash, const std::string& file) { files.push_back(file); first = hash; return files.size() < limit; };

	forEachInRange(fun, first, last);
	return files;
}

void DigestTree::forEachInRange(const std::function<bool(unsigned long long, const std::string&)>& fun, unsigned long long first, unsigned long long last) const
{
	_tree.enumerate(fun, first, last);
}

namespace {
	// for print()
	std::string printable(const std::tuple<unsigned long long, std::string>& fileData)
	{
		return std::get<1>(fileData);
	}
}

void DigestTree::print(int keywidth) const
{
	_tree.print(std::bind(&printable, _1), keywidth);
}
