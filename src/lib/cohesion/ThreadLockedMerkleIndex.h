/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IMerkleIndex.h"
class ISchedulerThread;

class ThreadLockedMerkleIndex : public IMerkleIndex
{
public:
	ThreadLockedMerkleIndex(IMerkleIndex& index, ISchedulerThread& scheduler);

	void add(const std::string& key, unsigned mirrors);
	void remove(const std::string& key, unsigned mirrors);

	void splitSection(const std::string& where);
	void cannibalizeSection(const std::string& where);

	const IMerkleTree& find(const std::string& id, unsigned mirrors) const;
	const IMerkleTree& randomTree() const;
	const IMerkleTree& unwantedTree() const;

protected:
	IMerkleIndex& _index;
	ISchedulerThread& _scheduler;
};
