/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "cohesion/IDigestKeys.h"
#include "cohesion/TreeId.h"
#include "util/CallHistory.h"
#include <deque>

class MockDigestTree : public IDigestKeys
{
public:
	MockDigestTree();
	const TreeId& id() const;

	void add(const std::string& key);
	void remove(const std::string& key);

	bool empty() const;
	MerklePoint top() const;
	std::deque<MerklePoint> diff(const MerklePoint& point) const;
	std::deque<std::string> enumerate(unsigned long long first, unsigned long long last, unsigned limit) const;

public:
	mutable CallHistory _history;
	TreeId _id;

	bool _empty;
	MerklePoint _top;
	std::deque<MerklePoint> _diff;
	std::deque<std::string> _enumerate;
};
