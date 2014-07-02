/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DataChain.h"

#include "common/MyMemberId.h"
#include "common/VectorClock.h"
#include "consistent_hashing/Hash.h"
#include "serialize/StringUtil.h"
using std::shared_ptr;
using std::vector;

bool DataChain::storeAsBestVersion(const std::shared_ptr<DataEntry>& entry)
{
	tbb::spin_rw_mutex::scoped_lock(_mutex);
	entry->md.version = bestVersion();
	entry->md.version.increment( MyMemberId() );
	return store_unlocked(entry);
}

bool DataChain::store(const std::shared_ptr<DataEntry>& entry, unsigned long long offset)
{
	if (entry->md.version.isDeleted())
		return storeDeleted(entry);

	VectorClock deletedVersion = entry->md.version;
	deletedVersion.markDeleted();

	tbb::spin_rw_mutex::scoped_lock(_mutex);
	for (auto it = _entries.begin(); it != _entries.end(); ++it)
	{
		VectorClock::COMPARE comp = entry->md.version.compare( (*it)->md.version );
		if (comp == VectorClock::EQUAL)
		{
			if (offset != (*it)->data.size())
				return false;

			entry->md.totalCopies = (*it)->md.totalCopies;
			entry->data = (*it)->data + entry->data;
			_entries.erase(it);
			break;
		}
		else if (comp == VectorClock::LESS_THAN && (*it)->md.supercede)
			return false;
		else if (deletedVersion.compare( (*it)->md.version ) == VectorClock::EQUAL)
			return false;
	}
	return store_unlocked(entry);
}

// already have lock
bool DataChain::store_unlocked(const std::shared_ptr<DataEntry>& entry)
{
	if (entry->md.supercede)
		clearLesser_unlocked(entry->md.version);

	_entries.push_back(entry);
	return true;
}

bool DataChain::storeDeleted(const std::shared_ptr<DataEntry>& entry)
{
	tbb::spin_rw_mutex::scoped_lock(_mutex);

	VectorClock candidate;
	for (vector< shared_ptr<DataEntry> >::iterator it = _entries.begin(); it != _entries.end(); ++it)
	{
		candidate = (*it)->md.version;
		VectorClock::COMPARE comp = entry->md.version.compare(candidate);

		// if this version exists (delete already exists), or is superceded, bail out
		if (comp == VectorClock::EQUAL)
			return false;
		else if (comp == VectorClock::LESS_THAN && (*it)->md.supercede)
			return false;

		// if we're deleting an existing entry, replace it with the deleted one.
		candidate.markDeleted();
		if (entry->md.version.compare(candidate) == VectorClock::EQUAL)
		{
			DataEntry& elem = *(*it);
			elem.data = entry->data;
			elem.md.version = candidate;
			return true;
		}
	}

	// else, just push_back this deleted entry.
	return store_unlocked(entry);
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

unsigned long long DataChain::summary() const
{
	tbb::spin_rw_mutex::scoped_lock(_mutex, false);
	unsigned long long sum = 0;
	for (auto it = _entries.begin(); it != _entries.end(); ++it)
	{
		std::string str = StringUtil::str((*it)->data.size()) + (*it)->md.version.toString();
		sum = sum xor Hash::compute(str).integer();
	}
	return sum;
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
