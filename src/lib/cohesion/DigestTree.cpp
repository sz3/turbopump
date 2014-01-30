/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DigestTree.h"

#include "common/MerklePoint.h"
#include "consistent_hashing/Hash.h"
#include <tuple>

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

void DigestTree::add(const std::string& key)
{
	// TODO: rather than storing "keyhash" as the value, we want something more interesting.
	//   this is what we're using to detect errors, after all. So: file + file version + crc?
	//   In any case this should become add(key, value)

	// TODO: worry about endianness of keyhash.
	// if different architectures can't agree on where a file hashes to... we're in trouble.
	// right now, it's derived from a char*, so it's mostly ok...
	//  except that the merkle_point::keybits don't act how you might expect...
	unsigned long long keyhash = Hash::compute(key).integer();
	_tree.insert(keyhash, keyhash, key);
}

void DigestTree::remove(const std::string& key)
{
	unsigned long long keyhash = Hash::compute(key).integer();
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

// for print()
std::ostream& operator<<(std::ostream& stream, const std::tuple<unsigned long long, std::string>& fileData)
{
	stream << std::get<1>(fileData);
	return stream;
}

void DigestTree::print(int keywidth) const
{
	_tree.print(keywidth);
}