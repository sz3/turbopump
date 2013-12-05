#include "BufferedSocketWriter.h"

#include "socket/IIpSocket.h"

// TODO: do virtual connections here?
// maybe only for buffered writes?
BufferedSocketWriter::BufferedSocketWriter(const std::shared_ptr<IIpSocket>& sock)
	: _sock(sock)
{
}

unsigned BufferedSocketWriter::maxPacketLength() const
{
	return 1500UL;
}

int BufferedSocketWriter::write(const char* buff, unsigned length)
{
	/*int res = 0;
	if (_buffer.size() + length > maxPacketLength())
		res = flush();
	while (length > 0)
	{
		if (length < maxPacketLength())
		{
			_buffer.append(buff, length);
			length = 0;
		}
		else
		{
			unsigned packetSize = maxPacketLength();
			length -= packetSize;
			res = _sock->send(buff, packetSize);
		}
	}
	return res;*/

	return _sock->send(buff, length);
}


int BufferedSocketWriter::flush()
{
	int res = _sock->send(_buffer.data(), _buffer.size());
	_buffer.clear();
	return res;
}
