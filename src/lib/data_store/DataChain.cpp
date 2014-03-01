/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DataChain.h"

#include "common/MyMemberId.h"
#include "common/VectorClock.h"
using std::shared_ptr;
using std::vector;

bool DataChain::storeAsBestVersion(const std::shared_ptr<DataEntry>& entry)
{
	tbb::spin_rw_mutex::scoped_lock(_mutex);
	entry->md.version = bestVersion();
	entry->md.version.increment( MyMemberId() );
	return store_unlocked(entry);
}

bool DataChain::store(const std::shared_ptr<DataEntry>& entry)
{
	tbb::spin_rw_mutex::scoped_lock(_mutex);
	return store_unlocked(entry);
}

// already have lock
bool DataChain::store_unlocked(const std::shared_ptr<DataEntry>& entry)
{
	if (entry->md.supercede)
		clearLesser_unlocked(entry->md.version);
	if (find_unlocked(entry->md.version) != _entries.end())
		return false;

	_entries.push_back(entry);
	return true;
}

unsigned DataChain::clearLesser_unlocked(const VectorClock& version)
{
	auto test_version_lesser = [&version](shared_ptr<DataEntry>& entry)
	{
		if (!entry)
			return false;
		return entry->md.version.compare(version) == VectorClock::LESS_THAN;
	};
	_entries.erase(std::remove_if(_entries.begin(), _entries.end(), test_version_lesser), _entries.end());
	return _entries.size();
}

unsigned DataChain::erase(const VectorClock& version)
{
	tbb::spin_rw_mutex::scoped_lock(_mutex);
	vector< shared_ptr<DataEntry> >::iterator elem = find_unlocked(version);
	if (elem != _entries.end())
		_entries.erase(elem);
	return _entries.size();
}

std::shared_ptr<DataEntry> DataChain::find(const VectorClock& version) const
{
	tbb::spin_rw_mutex::scoped_lock(_mutex, false);
	vector< shared_ptr<DataEntry> >::const_iterator elem = find_unlocked(version);
	if (elem == _entries.end())
		return NULL;
	return *elem;
}

std::vector< std::shared_ptr<DataEntry> >::iterator DataChain::find_unlocked(const VectorClock& version) const
{
	auto test_version_equals = [&version](shared_ptr<DataEntry>& entry)
	{
		if (!entry)
			return false;
		return entry->md.version.compare(version) == VectorClock::EQUAL;
	};
	return std::find_if(const_cast<vector<shared_ptr<DataEntry>>&>(_entries).begin(), const_cast<vector<shared_ptr<DataEntry>>&>(_entries).end(), test_version_equals);
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
