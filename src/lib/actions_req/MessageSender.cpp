/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MessageSender.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"
#include "wan_server/IBufferedConnectionWriter.h"
#include "wan_server/IPeerTracker.h"
#include <memory>
#include <sstream>
using std::shared_ptr;
using std::string;

MessageSender::MessageSender(IPeerTracker& peers)
	: _peers(peers)
{
}

bool MessageSender::sendMessage(const Peer& peer, const string& message)
{
	shared_ptr<IBufferedConnectionWriter> writer(_peers.getWriter(peer));
	if (!writer)
		return false;

	writer->write(peer.nextActionId(), message.data(), message.size());
	writer->flush();
	return true;
}

void MessageSender::merklePing(const Peer& peer, const string& treeid, const MerklePoint& point)
{
	sendMessage(peer, "merkle|tree=" + treeid + "|" + MerklePointSerializer::toString(point));
}

void MessageSender::merklePing(const Peer& peer, const string& treeid, const std::deque<MerklePoint>& points)
{
	std::string message = "merkle|tree=" + treeid + "|";
	message += MerklePointSerializer::toString(points.front());
	for (auto it = ++points.begin(); it != points.end(); ++it)
		message += "|" + MerklePointSerializer::toString(*it);
	sendMessage(peer, message);
}

void MessageSender::requestKeyRange(const Peer& peer, const string& treeid, unsigned long long first, unsigned long long last)
{
	std::stringstream msg;
	msg << "key-req|tree=" << treeid << " first=" << first << " last=" << last << "|";
	sendMessage(peer, msg.str());
}

void MessageSender::dropKey(const Peer& peer, const string& filename)
{
	sendMessage(peer, "drop|name=" + filename + "|");
}
