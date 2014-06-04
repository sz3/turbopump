/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "BufferedConnectionWriter.h"

#include "socket/IIpSocket.h"
#include <arpa/inet.h>
#include <iostream>
using std::mutex;
using std::lock_guard;
using std::string;

namespace {
	template <typename M>
	class unlock_guard
	{
	public:
		unlock_guard(M& myMutex)
			: _mutex(myMutex)
		{
			_mutex.unlock();
		}

		~unlock_guard()
		{
			_mutex.lock();
		}

	protected:
		M& _mutex;
	};
}

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

// TODO: needs to account for both async and sync writes.
// async writes fail if a flush is needed (try again later). Sync writes call flush.
// flush() needs to not occur under the mutex! Of course.
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

	{
		lock_guard<mutex> lock(_buffMutex);
		if (_buffer.size() > 0 && _buffer.size() + remaining > maxBufferSize)
			good = flush_internal(blocking);
	}

	while (good)
	{
		lock_guard<mutex> lock(_buffMutex);
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
		remaining -= packetSize;
		if (remaining == 0)
			break;
	}
	return length-remaining;
}

bool BufferedConnectionWriter::flush(bool blocking)
{
	lock_guard<mutex> lock(_buffMutex);
	return flush_internal(blocking);
}

bool BufferedConnectionWriter::flush_internal(bool blocking)
{
	if (blocking)
	{
		lock_guard<mutex> lock(_syncFlushMutex);
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
			_buffer = _buffer.substr(res);
		return false;
	}
}
