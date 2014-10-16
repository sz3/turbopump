/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
class IPeerCommandCenter;
class ILog;
class IMembership;
class ISocketWriter;

class PeerPacketHandler
{
public:
	PeerPacketHandler(const IMembership& membership, IPeerCommandCenter& center, ILog& logger);

	bool onPacket(ISocketWriter& writer, const char* buff, unsigned size);

protected:
	const IMembership& _membership;
	IPeerCommandCenter& _center;
	ILog& _logger;
};

