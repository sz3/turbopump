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

std::vector<std::string> MockHashRing::lookup(const std::string& filename, unsigned numWorkers) const
{
	_history.call("lookup", filename, numWorkers);
	return _workers;
}

std::string MockHashRing::toString() const
{
	_history.call("toString");
	return StringUtil::stlJoin(_workers);
}
