/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IHashRing.h"
#include "circular_map.h"

#include <string>
#include <vector>

class HashRing : public IHashRing
{
public:
	static std::string hash(const std::string& str);

public:
	void addWorker(const std::string& id);
	void removeWorker(const std::string& id);
	void growWorker(const std::string& name);
	void shrinkWorker(const std::string& name);

	std::vector<std::string> locations(const std::string& filename, unsigned numLocs = 5) const;
	std::vector<std::string> locationsFromHash(const std::string& hash, unsigned numLocs) const;
	std::string section(const std::string& filename) const;
	std::string lookup(const std::string& filename, std::vector<std::string>& locs, unsigned numLocs) const;
	std::string lookupFromHash(const std::string& hash, std::vector<std::string>& locs, unsigned numLocs) const;
	unsigned size() const;

	std::string toString() const;

protected:

protected:
	circular_map<std::string,std::string> _ring;
};

