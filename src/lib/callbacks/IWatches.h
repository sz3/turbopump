/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <functional>
#include <string>

// DynamicWatches?
class IWatches
{
public:
	virtual ~IWatches() {}

	virtual void watch(const std::string& name, const std::string& id, std::function<void()> onNotify) = 0;
	virtual bool unwatch(const std::string& name, const std::string& id) = 0;

	virtual bool notify(const std::string& name) const = 0;
};
