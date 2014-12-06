/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IReader.h"

class FileReader : public IReader
{
public:
	FileReader(const std::string& filename, unsigned long long offset = 0);
	~FileReader();

	bool good() const;

	int stream(IByteStream& sink);
	unsigned long long size() const;

protected:
	bool open(const std::string& filename, unsigned long long offset);
	void close();

protected:
	int _fd;
};
