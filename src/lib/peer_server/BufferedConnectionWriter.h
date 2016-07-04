/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IBufferedConnectionWriter.h"
#include "concurrent/conditional_lock_guard.h"
#include "concurrent/unlock_guard.h"
#include <memory>
#include <mutex>
#include <arpa/inet.h>

// TODO: encryption
template <typename Socket>
class BufferedConnectionWriter : public IBufferedConnectionWriter
{
public:
	BufferedConnectionWriter(const Socket& sock, unsigned packetsize=1472);

	unsigned capacity() const;
	int write(unsigned char virtid, const char* buffer, unsigned length, bool blocking);
	bool flush(bool blocking);
	bool close();

	std::string target() const;
	int handle() const;

protected:
	void pushBytes(unsigned char virtid, const char* buff, unsigned length);
	bool flush_internal(bool blocking);

	static unsigned findFirstTruncatedPacket(const char* buff, unsigned size);

protected:
	std::mutex _buffMutex;
	std::mutex _syncFlushMutex;
	std::string _buffer;
	unsigned _capacity;
	Socket _sock;
};

template <typename Socket>
BufferedConnectionWriter<Socket>::BufferedConnectionWriter(const Socket& sock, unsigned packetsize)
	: _sock(sock)
	, _capacity(packetsize-3) // will be -6 when encrypted?
{
	_buffer.reserve(packetsize);
}

template <typename Socket>
unsigned BufferedConnectionWriter<Socket>::capacity() const
{
	return _capacity;
}

template <typename Socket>
void BufferedConnectionWriter<Socket>::pushBytes(unsigned char virtid, const char* buff, unsigned length)
{
	unsigned short netlen = htons(length+1);
	_buffer.append( reinterpret_cast<const char*>(&netlen), 2 );
	_buffer.append( reinterpret_cast<const char*>(&virtid), 1 );
	_buffer.append(buff, length);
}

template <typename Socket>
int BufferedConnectionWriter<Socket>::write(unsigned char virtid, const char* buff, unsigned length, bool blocking)
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
	turbo::conditional_lock_guard<std::mutex> syncLock(_syncFlushMutex, blocking);
	std::lock_guard<std::mutex> buffLock(_buffMutex);
	if (_buffer.size() > 0 && _buffer.size() + remaining > maxBufferSize)
		good = flush_internal(blocking);

	while (good)
	{
		unsigned packetSize = _buffer.size() + remaining;
		if (packetSize >= maxBufferSize)
		{
			if (_buffer.size() < maxBufferSize)
			{
				packetSize = maxBufferSize - _buffer.size();
				pushBytes(virtid, buff, packetSize);
			}
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

template <typename Socket>
bool BufferedConnectionWriter<Socket>::flush(bool blocking)
{
	// get syncLock first, since we will be temporarily giving up buffLock...
	turbo::conditional_lock_guard<std::mutex> syncLock(_syncFlushMutex, blocking);
	std::lock_guard<std::mutex> buffLock(_buffMutex);
	return flush_internal(blocking);
}

template <typename Socket>
bool BufferedConnectionWriter<Socket>::flush_internal(bool blocking)
{
	if (blocking)
	{
		std::string buff(_buffer);
		_buffer.clear();
		turbo::unlock_guard<std::mutex> unlock(_buffMutex);

		// TODO: retry if sent < buff.size()?
		return _sock.send(buff.data(), buff.size());
	}
	else
	{
		int res = _sock.try_send(_buffer.data(), _buffer.size());
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

template <typename Socket>
unsigned BufferedConnectionWriter<Socket>::findFirstTruncatedPacket(const char* buff, unsigned size)
{
	unsigned i = 0;
	while (i < size)
	{
		unsigned short packetLen = ntohs( (*(buff+i+1) << 8) | *(buff+i) );
		if (packetLen+i+2 > size)
			return i;
		i += 2+packetLen;
	}
	return size;
}


template <typename Socket>
bool BufferedConnectionWriter<Socket>::close()
{
	return _sock.close();
}

template <typename Socket>
std::string BufferedConnectionWriter<Socket>::target() const
{
	return _sock.target();
}

template <typename Socket>
int BufferedConnectionWriter<Socket>::handle() const
{
	return _sock.handle();
}
