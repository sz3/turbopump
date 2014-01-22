/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/turbopump_defaults.h"
#include <string>

struct TreeId
{
	TreeId(const std::string& id="", unsigned mirrors=DEFAULT_MIRRORS)
		: id(id), mirrors(mirrors)
	{}

	std::string id;
	unsigned mirrors;
};
