/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>

class IBufferedConnectionWriter
{
public:
	virtual ~IBufferedConnectionWriter() {}

	virtual unsigned capacity() const = 0;
	virtual int write(unsigned char virtid, const char* buffer, unsigned length, bool blocking) = 0;
	virtual bool flush(bool blocking) = 0;
	virtual bool close() = 0;

	virtual std::string target() const = 0;
	virtual int handle() const = 0;
};
