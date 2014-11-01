/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ConsistentHashRing.h"

#include "Hash.h"
#include <ostream>

namespace {
	std::ostream& operator<<(std::ostream& outstream, const std::pair<std::string,std::string>& val)
	{
		outstream << val.first << "=" << val.second;
		return outstream;
	}
}
#include "serialize/str_join.h"

bool ConsistentHashRing::insert(const std::string& node, const std::string& peer)
{
	_ring[Hash(node).base64()] = peer;
	return true;
}

bool ConsistentHashRing::erase(const std::string& node)
{
	return _ring.erase(Hash(node).base64()) > 0;
}

std::vector<std::string> ConsistentHashRing::locations(const Hash& key, unsigned limit) const
{
	std::vector<std::string> locs;
	for (auto it = _ring.lower_bound(key.base64()); locs.size() < limit && it != _ring.end(); ++it)
		locs.push_back(it->second);
	return locs;
}

std::string ConsistentHashRing::nodeId(const Hash& key) const
{
	auto it = _ring.lower_bound(key.base64());
	if (it == _ring.end())
		return "";
	return it->first;
}

std::string ConsistentHashRing::toString() const
{
	return turbo::str::join(_ring);
}
