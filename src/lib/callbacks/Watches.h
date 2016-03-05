/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IWatches.h"
#include "libcuckoo/cuckoohash_map.hh"

class Watches : public IWatches
{
protected:
	using map_type = cuckoohash_map<std::string, std::function<void()>>;

public:
	void watch(const std::string& name, std::function<void()> onNotify);
	bool unwatch(const std::string& name);

	bool notify(const std::string& name) const;

protected:
	map_type _watches;
};
