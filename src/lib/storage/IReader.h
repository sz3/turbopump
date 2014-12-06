/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>
class IByteStream;

class IReader
{
public:
	virtual ~IReader() {}
	virtual bool good() const = 0;

	virtual int stream(IByteStream& sink) = 0;
	virtual unsigned long long size() const = 0;
};
