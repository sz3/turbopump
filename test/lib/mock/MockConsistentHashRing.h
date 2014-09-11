/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "hashing/IConsistentHashRing.h"
#include "util/CallHistory.h"

class MockConsistentHashRing : public IConsistentHashRing
{
public:
	bool insert(const std::string& node, const std::string& peer);
	bool erase(const std::string& node);

	std::vector<std::string> locations(const Hash& key, unsigned limit) const;
	std::string nodeId(const Hash& key) const;

	std::string toString() const;

public:
	mutable CallHistory _history;
	std::vector<std::string> _locations;
	std::string _nodeId;
};
