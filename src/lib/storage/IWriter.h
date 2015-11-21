/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
#include <string>
class IReader;

class IWriter
{
public:
	virtual ~IWriter() {}
	virtual bool good() const = 0;
	virtual unsigned long long position() const = 0;

	virtual int write(const char* buffer, unsigned length) = 0;
	virtual bool flush() = 0;
	virtual bool close() = 0;

	virtual IReader* reader() const = 0;

	virtual bool link(const std::string& source) { return false; }
};
