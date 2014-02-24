/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/turbopump_defaults.h"
#include <string>

struct WriteParams
{
	WriteParams()
		: mirror(0)
		, totalCopies(DEFAULT_MIRRORS)
	{}

	WriteParams(std::string filename, short mirror, short totalCopies, std::string version)
		: filename(filename)
		, mirror(mirror)
		, totalCopies(totalCopies)
		, version(version)
		, source("")
	{}

	std::string filename;
	short mirror;
	short totalCopies;
	std::string version;
	std::string source;
};
