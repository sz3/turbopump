/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IWriter.h"
#include <string>

class FileWriter : public IWriter
{
public:
	FileWriter(const std::string& filename);
	~FileWriter();

	bool good() const;
	unsigned long long position() const;

	int write(const char* buffer, unsigned length);
	bool flush();
	bool close();

	IReader* reader() const;

protected:
	bool open(const std::string& filename); //TODO: offset

protected:
	int _fd;
};
