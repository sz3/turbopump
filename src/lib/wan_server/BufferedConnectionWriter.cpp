#include "BufferedConnectionWriter.h"

#include "socket/IIpSocket.h"
#include <arpa/inet.h>
#include <iostream>

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
	std::cout << "pushBytes : " << (unsigned)virtid << ": " << (size_t)buff << ":" << length << std::endl;
	std::cout << "pushBytes again " << std::string(buff, length) << std::endl;
	unsigned short netlen = htons(length+1);
	_buffer.append( (const char*)&netlen, 2 );
	_buffer += virtid;
	_buffer.append(buff, length);
}

int BufferedConnectionWriter::write(unsigned char virtid, const char* buff, unsigned length)
{
	// [6 0 data1][12 1 data2-part1]
	// [34 1 data2-part2]
	// [30 1 data2-part3]

	int res = 0;
	if (_buffer.size() > 0 && _buffer.size() + length > capacity())
		res = flush();
	while(true)
	{
		unsigned packetSize = _buffer.size() + length;
		if (packetSize >= capacity())
		{
			packetSize = capacity();
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
	int res = _sock->send(_buffer.data(), _buffer.size());
	_buffer.clear();
	return res;
}
