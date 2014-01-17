/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IMerkleTree.h"
#include "data_structures/merkle_tree.h"

class MerkleTree : public IMerkleTree
{
public:
	void setId(const std::string& id);
	std::string id() const;

	void add(const std::string& key);
	void remove(const std::string& key);

	bool empty() const;
	MerklePoint top() const;
	std::deque<MerklePoint> diff(const MerklePoint& point) const;
	std::deque<std::string> enumerate(unsigned long long first, unsigned long long last, unsigned limit=50) const;

	void print(int keywidth=0) const; // for testing!

protected:
	std::string _id;
	merkle_tree<unsigned long long, unsigned long long, std::string> _tree;
};