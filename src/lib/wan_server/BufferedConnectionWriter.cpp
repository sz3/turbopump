/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "BufferedConnectionWriter.h"

#include "socket/IIpSocket.h"
#include <arpa/inet.h>
#include <iostream>
using std::recursive_mutex;
using std::lock_guard;

// TODO: do virtual connections here?
// maybe only for buffered writes?
BufferedConnectionWriter::BufferedConnectionWriter(const std::shared_ptr<IIpSocket>& sock, unsigned packetsize)
	: _sock(sock)
	, _capacity(packetsize-3) // will be -6 when encrypted?
{
	_buffer.reserve(packetsize);
}

unsigned BufferedConnectionWriter::capacity() const
{
	return _capacity;
}

void BufferedConnectionWriter::pushBytes(unsigned char virtid, const char* buff, unsigned length)
{
	unsigned short netlen = htons(length+1);
	_buffer.append( (const char*)&netlen, 2 );
	_buffer += virtid;
	_buffer.append(buff, length);
}

int BufferedConnectionWriter::write(unsigned char virtid, const char* buff, unsigned length)
{
	lock_guard<recursive_mutex> lock(_mutex);
	// [6 0 data1][12 1 data2-part1]
	// [34 1 data2-part2]
	// [30 1 data2-part3]

	unsigned maxBufferSize = capacity();
	int res = 0;
	if (_buffer.size() > 0 && _buffer.size() + length > maxBufferSize)
		res = flush();
	while(true)
	{
		unsigned packetSize = _buffer.size() + length;
		if (packetSize >= maxBufferSize)
		{
			packetSize = maxBufferSize;//-_buffer.size();
			pushBytes(virtid, buff, packetSize);
			res = flush();
		}
		else
		{
			packetSize = length;
			pushBytes(virtid, buff, packetSize);
		}

		buff += packetSize;
		length -= packetSize;
		if (length == 0)
			break;
	}
	return res;
}

int BufferedConnectionWriter::flush()
{
	lock_guard<recursive_mutex> lock(_mutex);
	int res = _sock->send(_buffer.data(), _buffer.size());
	_buffer.clear();
	return res;
}
