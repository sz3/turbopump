#pragma once

#include <string>

struct OrderedPacket
{
	unsigned seqnum;
	std::string buffer;

	bool operator<(const OrderedPacket& other) const
	{
		return seqnum > other.seqnum;
	}
};
