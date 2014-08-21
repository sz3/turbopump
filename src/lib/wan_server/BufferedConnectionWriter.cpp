/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "BufferedConnectionWriter.h"

#include "socket/ISocketWriter.h"
#include "mutex/conditional_lock_guard.h"
#include "mutex/unlock_guard.h"
#include <arpa/inet.h>
#include <iostream>
using std::mutex;
using std::lock_guard;
using std::string;

// TODO: do virtual connections here?
// maybe only for buffered writes?
BufferedConnectionWriter::BufferedConnectionWriter(const std::shared_ptr<ISocketWriter>& sock, unsigned packetsize)
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

int BufferedConnectionWriter::write(unsigned char virtid, const char* buff, unsigned length, bool blocking)
{
	// [6 0 data1][12 1 data2-part1]
	// [34 1 data2-part2]
	// [30 1 data2-part3]

	// if buffer is already "full", flush
	// while bytes, push (+flush) until length bytes have been consumed

	// if blocking, flush is async -- and we crash out with bytesWritten when it fails.

	// 1) test is buffer is full, flush
	//   a) async -- asyncMutex for buffer test + flush
	//   b) sync -- asyncMutex for buffer copy+clear + syncMutex for flush + release asyncMutex + flush()
	// 2) pushBytes -- asyncMutex for buffer write

	unsigned maxBufferSize = capacity();
	unsigned remaining = length;
	bool good = true;

	// get syncLock first, since we will be temporarily giving up buffLock...
	conditional_lock_guard<mutex> syncLock(_syncFlushMutex, blocking);
	lock_guard<mutex> buffLock(_buffMutex);
	if (_buffer.size() > 0 && _buffer.size() + remaining > maxBufferSize)
		good = flush_internal(blocking);

	while (good)
	{
		unsigned packetSize = _buffer.size() + remaining;
		if (packetSize >= maxBufferSize)
		{
			packetSize = maxBufferSize;//-_buffer.size();
			pushBytes(virtid, buff, packetSize);
			good = flush_internal(blocking);
		}
		else
		{
			packetSize = remaining;
			pushBytes(virtid, buff, packetSize);
		}

		buff += packetSize;
		remaining -= std::min(packetSize, remaining);
		if (remaining == 0)
			break;
	}
	return length-remaining;
}

bool BufferedConnectionWriter::flush(bool blocking)
{
	// get syncLock first, since we will be temporarily giving up buffLock...
	conditional_lock_guard<mutex> syncLock(_syncFlushMutex, blocking);
	lock_guard<mutex> buffLock(_buffMutex);
	return flush_internal(blocking);
}

bool BufferedConnectionWriter::flush_internal(bool blocking)
{
	if (blocking)
	{
		string buff(_buffer);
		_buffer.clear();
		unlock_guard<mutex> unlock(_buffMutex);

		// TODO: retry if sent < buff.size()?
		return _sock->send(buff.data(), buff.size());
	}
	else
	{
		int res = _sock->try_send(_buffer.data(), _buffer.size());
		if (res == _buffer.size())
		{
			_buffer.clear();
			return true;
		}
		if (res > 0)
			_buffer = _buffer.substr(findFirstTruncatedPacket(_buffer.data(), res));
		return false;
	}
}

unsigned BufferedConnectionWriter::findFirstTruncatedPacket(const char* buff, unsigned size)
{
	unsigned i = 0;
	while (i < size)
	{
		unsigned short packetLen = htons( *(unsigned short*)(buff+i) );
		if (packetLen+i+2 > size)
			return i;
		i += 2+packetLen;
	}
	return size;
}
