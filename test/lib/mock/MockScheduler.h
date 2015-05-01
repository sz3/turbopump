/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "concurrent/IScheduler.h"
#include "util/CallHistory.h"
#include <vector>

class MockScheduler : public IScheduler
{
public:
	void schedule(const std::function<void()>& fun, unsigned ms);
	void schedule_repeat(const std::function<void()>& fun, unsigned ms);

	void run();

public:
	CallHistory _history;
	std::vector< std::function<void()> > _funs;
};
