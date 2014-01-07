/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
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
