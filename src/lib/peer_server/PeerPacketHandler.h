/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
class IPeerCommandCenter;
class ILog;
class IKnowPeers;
class ISocketWriter;

class PeerPacketHandler
{
public:
	PeerPacketHandler(const IKnowPeers& membership, IPeerCommandCenter& center, ILog& logger);

	void onPacket(ISocketWriter& writer, const char* buff, unsigned size);

protected:
	const IKnowPeers& _membership;
	IPeerCommandCenter& _center;
	ILog& _logger;
};

