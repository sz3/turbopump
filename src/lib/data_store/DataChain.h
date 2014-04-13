/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "DataEntry.h"
#include "common/VectorClock.h"

#include "tbb/spin_rw_mutex.h"
#include <memory>
#include <vector>

class DataChain
{
public:
	bool storeAsBestVersion(const std::shared_ptr<DataEntry>& entry);
	bool store(const std::shared_ptr<DataEntry>& entry);
	bool markDeleted(const VectorClock& version);
	unsigned erase(const VectorClock& version);

	std::shared_ptr<DataEntry> find(const VectorClock& version) const;
	std::vector< std::shared_ptr<DataEntry> > entries() const;
	unsigned long long summary() const;

protected:
	unsigned clearLesser_unlocked(const VectorClock& version);
	bool store_unlocked(const std::shared_ptr<DataEntry>& entry);
	std::vector< std::shared_ptr<DataEntry> >::iterator find_unlocked(const VectorClock& version) const;

	// should be mergeVersions
	VectorClock bestVersion() const;

protected:
	mutable tbb::spin_rw_mutex _mutex;
	std::vector< std::shared_ptr<DataEntry> > _entries;
};

