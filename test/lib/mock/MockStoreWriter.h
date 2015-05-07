/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "storage/IWriter.h"
#include <string>

class MockStoreWriter : public IWriter
{
public:
	MockStoreWriter();

	bool good() const;
	unsigned long long position() const;

	int write(const char* buffer, unsigned length);
	bool flush();
	bool close();

	IReader* reader() const;

	static std::string calls();

public:
	unsigned long long _position = 0;
	std::string _reader;
};
