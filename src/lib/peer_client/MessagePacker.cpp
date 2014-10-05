/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MessagePacker.h"

#include <arpa/inet.h>
using std::string;

void MessagePacker::pack(char* buffer, unsigned char id, const char* packet, unsigned size) const
{
	unsigned short netlen = htons(size+1);
	std::copy( (const char*)&netlen, (const char*)(&netlen)+2, buffer );
	std::copy( &id, (&id)+1, buffer+2 );
	std::copy( packet, packet+size, buffer+3 );
}

string MessagePacker::package(unsigned char id, const char* packet, unsigned size) const
{
	string res;
	res.resize(size+3);
	pack(&res[0], id, packet, size);
	return res;
}

string MessagePacker::package(unsigned char id, const string& packet) const
{
	return package(id, packet.data(), packet.size());
}
