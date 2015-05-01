/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockScheduler.h"

void MockScheduler::schedule(const std::function<void()>& fun, unsigned ms)
{
	_history.call("schedule", ms);
	_funs.push_back(fun);
}

void MockScheduler::schedule_repeat(const std::function<void()>& fun, unsigned ms)
{
	_history.call("schedule_repeat", ms);
	_funs.push_back(fun);
}

void MockScheduler::run()
{
	for (std::function<void()>& fun : _funs)
		fun();
}
