/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "event/ISchedulerThread.h"
#include "util/CallHistory.h"
#include <vector>

class MockSchedulerThread : public ISchedulerThread
{
public:
	void schedule(const std::function<void()>& fun, unsigned millis);
	void schedulePeriodic(const std::function<void()>& fun, unsigned millis);

	void run();

public:
	CallHistory _history;
	std::vector< std::function<void()> > _funs;
};
