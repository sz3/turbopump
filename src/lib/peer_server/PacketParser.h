/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
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
