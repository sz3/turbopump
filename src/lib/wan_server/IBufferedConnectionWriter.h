#pragma once

class IBufferedConnectionWriter
{
public:
	virtual ~IBufferedConnectionWriter() {}

	virtual unsigned capacity() const = 0;
	virtual int write(unsigned char virtid, const char* buffer, unsigned length) = 0;
	virtual int flush() = 0;
};
