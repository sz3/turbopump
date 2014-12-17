/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IReader.h"

class FileReader : public IReader
{
public:
	FileReader(int fd);
	FileReader(const std::string& filename, unsigned long long offset=0);
	~FileReader();

	bool good() const;
	unsigned long long size() const;
	std::string attribute(const char* key) const;

	bool setPosition(unsigned long long offset);
	int stream(IByteStream& sink);

protected:
	bool open(const std::string& filename, unsigned long long offset);
	void close();

protected:
	int _fd;
};
