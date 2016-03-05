/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "callbacks/IWatches.h"
#include "util/CallHistory.h"

class MockWatches : public IWatches
{
public:
	void watch(const std::string& name, std::function<void()> onNotify);
	bool unwatch(const std::string& name);

	bool notify(const std::string& name) const;

public:
	mutable CallHistory _history;
};

inline void MockWatches::watch(const std::string& name, std::function<void()> onNotify)
{
	_history.call("watch", name);
}

inline bool MockWatches::unwatch(const std::string& name)
{
	_history.call("unwatch", name);
	return true;
}

inline bool MockWatches::notify(const std::string& name) const
{
	_history.call("notify", name);
	return true;
}

