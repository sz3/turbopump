/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ThreadLockedMerkleIndex.h"

#include "event/ISchedulerThread.h"

ThreadLockedMerkleIndex::ThreadLockedMerkleIndex(IMerkleIndex& index, ISchedulerThread& scheduler)
	: _index(index)
	, _scheduler(scheduler)
{
}

void ThreadLockedMerkleIndex::add(const std::string& key)
{
	_scheduler.schedule( bind(&IMerkleIndex::add, std::ref(_index), key), 0 );
}

void ThreadLockedMerkleIndex::remove(const std::string& key)
{
	_scheduler.schedule( bind(&IMerkleIndex::remove, std::ref(_index), key), 0 );
}

const IMerkleTree& ThreadLockedMerkleIndex::find(const std::string& id) const
{
	return _index.find(id);
}

const IMerkleTree& ThreadLockedMerkleIndex::randomTree() const
{
	return _index.randomTree();
}

const IMerkleTree& ThreadLockedMerkleIndex::unwantedTree() const
{
	return _index.unwantedTree();
}
