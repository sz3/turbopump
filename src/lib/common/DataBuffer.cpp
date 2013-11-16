#include "DataBuffer.h"

using std::string;

DataBuffer DataBuffer::Null()
{
	static DataBuffer null(NULL, 0);
	return null;
}

DataBuffer::DataBuffer(const char* buff, unsigned size)
	: _buffer(buff)
	, _size(size)
{
}

string DataBuffer::str() const
{
	return string(_buffer, _size);
}

const char* DataBuffer::buffer() const
{
	return _buffer;
}

unsigned DataBuffer::size() const
{
	return _size;
}

void DataBuffer::skip(unsigned bytes)
{
	if (bytes > _size)
		bytes = _size;
	_buffer += bytes;
	_size -= bytes;
}

string DataBuffer::read(unsigned bytes)
{
	if (bytes > _size)
		bytes = _size;

	const char* buff = _buffer;
	_buffer += bytes;
	_size -= bytes;
	return string(buff, bytes);
}
