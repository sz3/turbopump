/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ThreadLockedKeyTabulator.h"

#include "event/ISchedulerThread.h"

ThreadLockedKeyTabulator::ThreadLockedKeyTabulator(IKeyTabulator& index, ISchedulerThread& scheduler)
	: _index(index)
	, _scheduler(scheduler)
{
}

void ThreadLockedKeyTabulator::add(const std::string& key, unsigned mirrors)
{
	_scheduler.schedule( bind(&IKeyTabulator::add, std::ref(_index), key, mirrors), 0 );
}

void ThreadLockedKeyTabulator::remove(const std::string& key, unsigned mirrors)
{
	_scheduler.schedule( bind(&IKeyTabulator::remove, std::ref(_index), key, mirrors), 0 );
}

void ThreadLockedKeyTabulator::splitSection(const std::string& where)
{
	_scheduler.schedule( bind(&IKeyTabulator::splitSection, std::ref(_index), where), 0 );
}

void ThreadLockedKeyTabulator::cannibalizeSection(const std::string& where)
{
	_scheduler.schedule( bind(&IKeyTabulator::cannibalizeSection, std::ref(_index), where), 0 );
}

const IDigestKeys& ThreadLockedKeyTabulator::find(const std::string& id, unsigned mirrors) const
{
	return _index.find(id, mirrors);
}

const IDigestKeys& ThreadLockedKeyTabulator::randomTree() const
{
	return _index.randomTree();
}

const IDigestKeys& ThreadLockedKeyTabulator::unwantedTree() const
{
	return _index.unwantedTree();
}
