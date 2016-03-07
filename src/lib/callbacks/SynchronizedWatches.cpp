/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "SynchronizedWatches.h"

#include "concurrent/IScheduler.h"

SynchronizedWatches::SynchronizedWatches(IWatches& watches, IScheduler& scheduler)
	: _watches(watches)
	, _scheduler(scheduler)
{
}

void SynchronizedWatches::watch(const std::string& name, const std::string& wid, std::function<void()> onNotify)
{
	_scheduler.execute( bind(&IWatches::watch, std::ref(_watches), name, wid, onNotify) );
}

bool SynchronizedWatches::unwatch(const std::string& name, const std::string& wid)
{
	_scheduler.execute( bind(&IWatches::unwatch, std::ref(_watches), name, wid) );
	return true;
}

bool SynchronizedWatches::notify(const std::string& name) const
{
	_scheduler.execute( bind(&IWatches::notify, std::ref(_watches), name) );
	return true;
}
