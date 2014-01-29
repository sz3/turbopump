/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/MerklePoint.h"
#include <deque>
#include <string>
class TreeId;

// tracks files that belong to a given HashRing section.
// e.g. "this tree should be synchronized with AAA and BBB, who both track section XYZ"

class IDigestKeys
{
public:
	virtual ~IDigestKeys() {}
	virtual const TreeId& id() const = 0;

	virtual void add(const std::string& key) = 0;
	virtual void remove(const std::string& key) = 0;

	virtual bool empty() const = 0;
	virtual MerklePoint top() const = 0;
	virtual std::deque<MerklePoint> diff(const MerklePoint& point) const = 0;
	virtual std::deque<std::string> enumerate(unsigned long long first, unsigned long long last, unsigned limit=50) const = 0;
};
