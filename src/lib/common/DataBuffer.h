/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>

class DataBuffer
{
public:
	static DataBuffer Null();

	DataBuffer(const char* buff, unsigned size);
	DataBuffer& operator=(const DataBuffer& other);

	std::string str() const;
	const char* buffer() const;
	unsigned size() const;

	void skip(unsigned bytes);
	std::string read(unsigned bytes);

protected:
	const char* _buffer;
	unsigned _size;
};
