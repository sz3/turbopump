/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Watches.h"
#include "util/random.h"

void Watches::watch(const std::string& name, const std::string& id, std::function<void()> onNotify)
{
	watch_key_type key = {name, id};
	_watches[key] = onNotify;
}

bool Watches::unwatch(const std::string& name, const std::string& id)
{
	watch_key_type key = {name, id};
	return _watches.erase(key);
}

bool Watches::notify(const std::string& name) const
{
	watch_key_type key = {name, ""};
	size_t bucketIdx = _watches.bucket(key);
	map_type::const_local_iterator it = _watches.begin(bucketIdx);
	map_type::const_local_iterator end = _watches.end(bucketIdx);
	if (it == end)
		return false;

	for (; it != end; ++it)
		if (it->first.name == name)
			it->second();
	return true;
}
