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
	std::shared_ptr<DataEntry> create();
	std::shared_ptr<DataEntry> createVersion(const VectorClock& version);
	std::vector< std::shared_ptr<DataEntry> > entries() const;

protected:
	std::shared_ptr<DataEntry> create_unlocked(const VectorClock& version);
	VectorClock bestVersion() const;

protected:
	mutable tbb::spin_rw_mutex _mutex;
	std::vector< std::shared_ptr<DataEntry> > _entries;
};

