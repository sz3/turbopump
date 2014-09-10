#include "MockConsistentHashRing.h"

#include "consistent_hashing/Hash.h"

bool MockConsistentHashRing::insert(const std::string& node, const std::string& peer)
{
	_history.call("insert", node, peer);
	return true;
}

bool MockConsistentHashRing::erase(const std::string& node)
{
	_history.call("erase", node);
	return true;
}

std::vector<std::string> MockConsistentHashRing::locations(const Hash& key, unsigned limit) const
{
	_history.call("locations", key.base64(), limit);
	return _locations;
}

std::string MockConsistentHashRing::nodeId(const Hash& key) const
{
	_history.call("nodeId", key.base64());
	return _nodeId;
}

std::string MockConsistentHashRing::toString() const
{
	_history.call("toString");
	return "mockCHR";
}
