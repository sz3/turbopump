#pragma once

class DataBuffer;

class PacketParser
{
public:
	PacketParser(DataBuffer& buff);

	bool getNext(unsigned char& virtid, DataBuffer& buff);

protected:
	DataBuffer& _buff;
};
