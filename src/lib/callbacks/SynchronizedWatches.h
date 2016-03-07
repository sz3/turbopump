/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IWatches.h"
class IScheduler;

class SynchronizedWatches : public IWatches
{
public:
	SynchronizedWatches(IWatches& watches, IScheduler& scheduler);

	void watch(const std::string& name, const std::string& wid, std::function<void()> onNotify);
	bool unwatch(const std::string& name, const std::string& wid);

	bool notify(const std::string& name) const;

protected:
	IWatches& _watches;
	IScheduler& _scheduler;
};
