/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IDigestKeys.h"
#include "TreeId.h"
#include "merkle_tree/merkle_tree.h"

class DigestTree : public IDigestKeys
{
public:
	static const DigestTree& null();

public:
	void setId(const TreeId& id);
	const TreeId& id() const;

	void add(const std::string& key, unsigned long long hash);
	void update(const std::string& key, unsigned long long value);
	void remove(const std::string& key);

	bool empty() const;
	MerklePoint top() const;
	MerkleDiffResult diff(const MerklePoint& point) const;
	std::deque<std::string> enumerate(unsigned long long first, unsigned long long last, unsigned limit=50) const;
	void forEachInRange(const std::function<bool(unsigned long long, unsigned long long, const std::string&)>& fun, unsigned long long first, unsigned long long last) const;

	void print(int keywidth=0) const; // for testing!

protected:
	TreeId _id;
	// sorted as big endian (because merkle_tree does char-wise comparisons.) Just FYI!
	turbo::merkle_tree<unsigned long long, unsigned long long, std::string> _tree;
};
