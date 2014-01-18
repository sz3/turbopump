/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "consistent_hashing/IHashRing.h"
#include "util/CallHistory.h"
#include <string>
#include <vector>

class MockHashRing : public IHashRing
{
public:
	void addWorker(const std::string& id);
	void removeWorker(const std::string& id);

	std::vector<std::string> locations(const std::string& filename, unsigned numLocs = 5) const;
	std::vector<std::string> locationsFromHash(const std::string& hash, unsigned numLocs) const;
	std::string section(const std::string& filename) const;
	std::string lookup(const std::string& filename, std::vector<std::string>& locs, unsigned numLocs) const;

	std::string toString() const;

public:
	std::vector<std::string> _workers;
	mutable CallHistory _history;
};
