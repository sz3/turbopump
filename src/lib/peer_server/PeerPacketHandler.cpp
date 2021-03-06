/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "PeerPacketHandler.h"

#include "IPeerCommandCenter.h"
#include "logging/ILog.h"
#include "membership/IKnowPeers.h"
#include "socket/ISocketWriter.h"
#include "socket/socket_address.h"
using std::string;

PeerPacketHandler::PeerPacketHandler(const IKnowPeers& membership, IPeerCommandCenter& center, ILog& logger)
	: _membership(membership)
	, _center(center)
	, _logger(logger)
{
}

void PeerPacketHandler::onPacket(ISocketWriter& writer, const char* buff, unsigned size)
{
	// is the message from a valid peer?
	std::shared_ptr<Peer> peer = _membership.lookupAddr(writer.endpoint());
	if (!peer)
	{
		_logger.logWarn("rejecting packet from unknown host " + writer.endpoint().toString());
		return;
	}

	if (buff == NULL)
	{
		_center.dismiss(peer);
		return;
	}

	if (size == 0)
		return;

	string buffer(buff, size); // decrypt here
	_center.run(peer, buffer);
}
