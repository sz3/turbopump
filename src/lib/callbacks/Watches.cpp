/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Watches.h"

void Watches::watch(const std::string& name, std::function<void()> onNotify)
{
	_watches.insert(name, onNotify);
}

bool Watches::unwatch(const std::string& name)
{
	return _watches.erase(name);
}

bool Watches::notify(const std::string& name) const
{
	std::function<void()> onNotify;
	if (!_watches.find(name, onNotify))
		return false;

	onNotify();
	return true;
}
