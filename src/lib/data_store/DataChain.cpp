/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DataChain.h"

#include "common/MyMemberId.h"
#include "common/VectorClock.h"
using std::shared_ptr;

std::shared_ptr<DataEntry> DataChain::create()
{
	// get version increment from a singleton
	tbb::spin_rw_mutex::scoped_lock(_mutex);
	VectorClock version = bestVersion();
	version.increment( MyMemberId() );
	return create_unlocked(version);
}

std::shared_ptr<DataEntry> DataChain::createVersion(const VectorClock& version)
{
	tbb::spin_rw_mutex::scoped_lock(_mutex);
	return create_unlocked(version);
}

// already have lock
std::shared_ptr<DataEntry> DataChain::create_unlocked(const VectorClock& version)
{
	std::shared_ptr<DataEntry> entry(new DataEntry);
	entry->md.version = version;
	_entries.push_back(entry);
	return entry;
}

std::vector< std::shared_ptr<DataEntry> > DataChain::entries() const
{
	tbb::spin_rw_mutex::scoped_lock(_mutex, false);
	return _entries;
}

// should already have lock
VectorClock DataChain::bestVersion() const
{
	tbb::spin_rw_mutex::scoped_lock(_mutex, false);
	VectorClock aggregate;
	for (auto it = _entries.begin(); it != _entries.end(); ++it)
		aggregate.merge( (*it)->md.version );
	return aggregate;
}
