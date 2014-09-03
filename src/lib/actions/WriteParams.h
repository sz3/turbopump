/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/turbopump_defaults.h"
#include <memory>
#include <string>
class ConnectionWriteStream;

// TODO: move ConnectionWriteStream and isComplete (write state information, that is) into higher-level class holding a WriteParams.
// WriteParams should be "pure"

struct WriteParams //: public KeyMetadata
{
	WriteParams()
		: mirror(0)
		, totalCopies(DEFAULT_MIRRORS)
		, offset(0)
		, isComplete(false)
	{}

	WriteParams(std::string filename, short mirror, short totalCopies, std::string version, unsigned long long offset)
		: filename(filename)
		, mirror(mirror)
		, totalCopies(totalCopies)
		, version(version)
		, offset(0)
		, isComplete(false)
	{}

	std::string filename;
	short mirror;
	short totalCopies;
	std::string version;
	std::string source;
	unsigned long long offset;

	std::shared_ptr<ConnectionWriteStream> outstream;
	bool isComplete;
};
