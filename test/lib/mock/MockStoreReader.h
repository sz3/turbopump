/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "storage/IReader.h"
#include <string>

class MockStoreReader : public IReader
{
public:
	bool good() const;
	unsigned long long size() const;

	bool setPosition(unsigned long long offset);
	int stream(IByteStream& sink);

	std::string calls();

public:
	unsigned long long _size = 10;
};
