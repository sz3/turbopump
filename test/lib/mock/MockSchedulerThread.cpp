/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockSchedulerThread.h"

void MockSchedulerThread::schedule(const std::function<void()>& fun, unsigned millis)
{
	_history.call("schedule", millis);
	_funs.push_back(fun);
}

void MockSchedulerThread::schedulePeriodic(const std::function<void()>& fun, unsigned millis)
{
	_history.call("schedulePeriodic", millis);
	_funs.push_back(fun);
}

void MockSchedulerThread::run()
{
	for (std::function<void()>& fun : _funs)
		fun();
}
