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

	std::vector<std::string> lookup(const std::string& filename, unsigned numWorkers = 5) const;

public:
	std::vector<std::string> _workers;
	mutable CallHistory _history;
};
