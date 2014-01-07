/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IHashRing.h"
#include "circular_map.h"

#include <string>
#include <vector>
using std::string;

class HashRing : public IHashRing
{
public:
	static string hash(const string& str);

public:
	void addWorker(const string& id);
	void removeWorker(const string& id);
	void growWorker(const string& name);
	void shrinkWorker(const string& name);

	std::vector<string> lookup(const string& filename, unsigned numWorkers = 5) const;
	unsigned size() const;

protected:
	circular_map<string,string> _ring;
};

