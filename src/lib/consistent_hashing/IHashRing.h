/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>
#include <vector>

class IHashRing
{
public:
	virtual ~IHashRing() {}

	virtual void addWorker(const std::string& id) = 0;
	virtual void removeWorker(const std::string& id) = 0;

	// I think this nees to return the primary location (hash) as well -- so, a richer data structure (string + vector)
	// otherwise we won't know how to arrange stuff for merkle-ing
	virtual std::vector<std::string> lookup(const std::string& filename, unsigned numWorkers = 5) const = 0;
};
