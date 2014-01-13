/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>

struct KeyMetadata
{
	KeyMetadata()
		: mirror(0)
		, totalCopies(2)
	{}

	KeyMetadata(std::string filename, short mirror, short totalCopies)
		: filename(filename)
		, mirror(mirror)
		, totalCopies(totalCopies)
		, source("")
	{}

	std::string filename;
	short mirror;
	short totalCopies;
	std::string source;
};
