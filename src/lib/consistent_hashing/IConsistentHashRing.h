/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>
#include <vector>
class Hash;

class IConsistentHashRing
{
public:
	virtual ~IConsistentHashRing() {}

	virtual bool insert(const std::string& node, const std::string& peer) = 0;
	virtual bool erase(const std::string& node) = 0;

	virtual std::vector<std::string> locations(const Hash& key, unsigned limit) const = 0;
	virtual std::string nodeId(const Hash& key) const = 0;

	virtual std::string toString() const = 0;
};
