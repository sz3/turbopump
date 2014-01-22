/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>
class IMerkleTree;

class IMerkleRing
{
public:
	virtual ~IMerkleRing() {}

	virtual void add(const std::string& key) = 0;
	virtual void remove(const std::string& key) = 0;

	virtual void splitSection(const std::string& where) = 0;
	virtual void cannibalizeSection(const std::string& where) = 0;

	virtual const IMerkleTree& find(const std::string& id) const = 0;
	virtual const IMerkleTree& randomTree() const = 0;
	virtual const IMerkleTree& unwantedTree() const = 0;
};
