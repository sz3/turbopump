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

	virtual std::vector<std::string> locations(const std::string& filename, unsigned numLocs = 5) const = 0;
	virtual std::vector<std::string> locationsFromHash(const std::string& hash, unsigned numLocs) const = 0;
	virtual std::string section(const std::string& filename) const = 0;

	virtual std::string toString() const = 0;
};
