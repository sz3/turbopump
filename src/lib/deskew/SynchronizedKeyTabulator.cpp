/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "SynchronizedKeyTabulator.h"

#include "concurrent/IScheduler.h"

SynchronizedKeyTabulator::SynchronizedKeyTabulator(IKeyTabulator& index, IScheduler& scheduler)
	: _index(index)
	, _scheduler(scheduler)
{
}

void SynchronizedKeyTabulator::update(const std::string& key, unsigned long long value, unsigned mirrors)
{
	_scheduler.execute( bind(&IKeyTabulator::update, std::ref(_index), key, value, mirrors) );
}

void SynchronizedKeyTabulator::remove(const std::string& key, unsigned mirrors)
{
	_scheduler.execute( bind(&IKeyTabulator::remove, std::ref(_index), key, mirrors) );
}

void SynchronizedKeyTabulator::splitSection(const std::string& where)
{
	_scheduler.execute( bind(&IKeyTabulator::splitSection, std::ref(_index), where) );
}

void SynchronizedKeyTabulator::cannibalizeSection(const std::string& where)
{
	_scheduler.execute( bind(&IKeyTabulator::cannibalizeSection, std::ref(_index), where) );
}

const IDigestKeys& SynchronizedKeyTabulator::find(const std::string& id, unsigned mirrors) const
{
	return _index.find(id, mirrors);
}

const IDigestKeys& SynchronizedKeyTabulator::randomTree() const
{
	return _index.randomTree();
}

const IDigestKeys& SynchronizedKeyTabulator::unwantedTree() const
{
	return _index.unwantedTree();
}
