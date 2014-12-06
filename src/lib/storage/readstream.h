/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/KeyMetadata.h"
#include <memory>
#include <string>
class IByteStream;
class IReader;

class readstream
{
public:
	readstream();
	readstream(const std::shared_ptr<IReader>& reader, const KeyMetadata& md);

	operator bool() const;
	bool good() const;

	int stream(IByteStream& sink);

	unsigned long long size() const;
	unsigned long long digest() const;
	unsigned short mirrors() const;
	std::string version() const;

protected:
	std::shared_ptr<IReader> _reader;
	KeyMetadata _md;
};
