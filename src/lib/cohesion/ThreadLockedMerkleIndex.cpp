/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ThreadLockedMerkleIndex.h"

#include "event/ISchedulerThread.h"

ThreadLockedMerkleIndex::ThreadLockedMerkleIndex(IMerkleIndex& index, ISchedulerThread& scheduler)
	: _index(index)
	, _scheduler(scheduler)
{
}

void ThreadLockedMerkleIndex::add(const std::string& key, unsigned mirrors)
{
	_scheduler.schedule( bind(&IMerkleIndex::add, std::ref(_index), key, mirrors), 0 );
}

void ThreadLockedMerkleIndex::remove(const std::string& key, unsigned mirrors)
{
	_scheduler.schedule( bind(&IMerkleIndex::remove, std::ref(_index), key, mirrors), 0 );
}

void ThreadLockedMerkleIndex::splitSection(const std::string& where)
{
	_scheduler.schedule( bind(&IMerkleIndex::splitSection, std::ref(_index), where), 0 );
}

void ThreadLockedMerkleIndex::cannibalizeSection(const std::string& where)
{
	_scheduler.schedule( bind(&IMerkleIndex::cannibalizeSection, std::ref(_index), where), 0 );
}

const IMerkleTree& ThreadLockedMerkleIndex::find(const std::string& id, unsigned mirrors) const
{
	return _index.find(id, mirrors);
}

const IMerkleTree& ThreadLockedMerkleIndex::randomTree() const
{
	return _index.randomTree();
}

const IMerkleTree& ThreadLockedMerkleIndex::unwantedTree() const
{
	return _index.unwantedTree();
}
