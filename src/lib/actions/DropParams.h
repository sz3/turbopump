/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/turbopump_defaults.h"
#include <string>

struct DropParams
{
	DropParams()
		: totalCopies(DEFAULT_MIRRORS)
	{}

	std::string filename;
	short totalCopies;
};
