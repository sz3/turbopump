/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IMerkleIndex.h"
class ISchedulerThread;

class ThreadLockedMerkleIndex : public IMerkleIndex
{
public:
	ThreadLockedMerkleIndex(IMerkleIndex& index, ISchedulerThread& scheduler);

	void add(const std::string& key);
	void remove(const std::string& key);

	const IMerkleTree& find(const std::string& id) const;
	const IMerkleTree& randomTree() const;
	const IMerkleTree& unwantedTree() const;

protected:
	IMerkleIndex& _index;
	ISchedulerThread& _scheduler;
};
