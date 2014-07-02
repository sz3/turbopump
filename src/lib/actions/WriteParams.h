/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/turbopump_defaults.h"
#include <string>

struct WriteParams //: public KeyMetadata
{
	WriteParams()
		: mirror(0)
		, totalCopies(DEFAULT_MIRRORS)
		, offset(0)
	{}

	WriteParams(std::string filename, short mirror, short totalCopies, std::string version, unsigned long long offset)
		: filename(filename)
		, mirror(mirror)
		, totalCopies(totalCopies)
		, version(version)
		, source("")
		, offset(0)
	{}

	std::string filename;
	short mirror;
	short totalCopies;
	std::string version;
	std::string source;
	unsigned long long offset;
};
