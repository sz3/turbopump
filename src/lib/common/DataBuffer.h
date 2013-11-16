#pragma once

#include <string>

class DataBuffer
{
public:
	static DataBuffer Null();

	DataBuffer(const char* buff, unsigned size);

	std::string str() const;
	const char* buffer() const;
	unsigned size() const;

	void skip(unsigned bytes);
	std::string read(unsigned bytes);

protected:
	const char* _buffer;
	unsigned _size;
};
