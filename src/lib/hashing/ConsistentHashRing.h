/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

// http://www.martinbroadhurst.com/Consistent-Hash-Ring.html
// http://blogs.msdn.com/b/csliu/archive/2009/09/17/consistent-hashing-theory-implementation.aspx
// http://docs.openstack.org/developer/swift/ring.html

#include "IConsistentHashRing.h"
#include "consistent_hashing/circular_map.h"

class ConsistentHashRing : public IConsistentHashRing
{
public:
	bool insert(const std::string& node, const std::string& peer);
	bool erase(const std::string& node);

	std::vector<std::string> locations(const Hash& key, unsigned limit) const;
	std::string nodeId(const Hash& key) const;

	std::string toString() const;

protected:
	turbo::circular_map<std::string, std::string> _ring;
};
