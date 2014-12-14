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
