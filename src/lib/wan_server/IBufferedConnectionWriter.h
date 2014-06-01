/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

class IBufferedConnectionWriter
{
public:
	virtual ~IBufferedConnectionWriter() {}

	virtual unsigned capacity() const = 0;
	virtual int write(unsigned char virtid, const char* buffer, unsigned length) = 0;
	virtual int flush() = 0;

	virtual void ensureDelivery_inc() = 0;
	virtual void ensureDelivery_dec() = 0;
};
