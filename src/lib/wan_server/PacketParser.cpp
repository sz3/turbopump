#include "PacketParser.h"

#include "common/DataBuffer.h"
#include <arpa/inet.h>

PacketParser::PacketParser(DataBuffer& buff)
	: _buff(buff)
{
}

bool PacketParser::getNext(unsigned char& virtid, DataBuffer& next)
{
	if (_buff.size() < 3)
		return false;

	unsigned short length = ntohs(*(unsigned short*)_buff.buffer())-1;
	_buff.skip(2);
	virtid = *_buff.buffer();
	_buff.skip(1);
	if (length > _buff.size())
		return false;
	next = DataBuffer(_buff.buffer(), length);
	_buff.skip(length);
	return true;
}
