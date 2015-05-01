/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IKeyTabulator.h"
class IScheduler;

class ThreadLockedKeyTabulator : public IKeyTabulator
{
public:
	ThreadLockedKeyTabulator(IKeyTabulator& index, IScheduler& scheduler);

	void update(const std::string& key, unsigned long long value, unsigned mirrors);
	void remove(const std::string& key, unsigned mirrors);

	void splitSection(const std::string& where);
	void cannibalizeSection(const std::string& where);

	const IDigestKeys& find(const std::string& id, unsigned mirrors) const;
	const IDigestKeys& randomTree() const;
	const IDigestKeys& unwantedTree() const;

protected:
	IKeyTabulator& _index;
	IScheduler& _scheduler;
};
