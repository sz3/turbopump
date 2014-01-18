/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockHashRing.h"

#include "serialize/StringUtil.h"

void MockHashRing::addWorker(const std::string& id)
{
	_history.call("addWorker", id);
}

void MockHashRing::removeWorker(const std::string& id)
{
	_history.call("removeWorker", id);
}

std::vector<std::string> MockHashRing::locations(const std::string& filename, unsigned numWorkers) const
{
	_history.call("locations", filename, numWorkers);
	return _workers;
}

std::vector<std::string> MockHashRing::locationsFromHash(const std::string& hash, unsigned numWorkers) const
{
	_history.call("locationsFromHash", hash, numWorkers);
	return _workers;
}

std::string MockHashRing::section(const std::string& filename) const
{
	_history.call("section", filename);
	return _workers.empty()? "" : _workers.front();
}

std::string MockHashRing::lookup(const std::string& filename, std::vector<std::string>& locs, unsigned numLocs) const
{
	_history.call("lookup", filename);
	locs = _workers;
	return _workers.empty()? "" : _workers.front();
}

std::string MockHashRing::toString() const
{
	_history.call("toString");
	return StringUtil::join(_workers);
}
