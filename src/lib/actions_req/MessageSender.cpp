/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MessageSender.h"

#include "cohesion/TreeId.h"
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

void MessageSender::digestPing(const Peer& peer, const TreeId& treeid, const MerklePoint& point)
{
	std::stringstream msg;
	msg << "sync|tree=" << treeid.id << " n=" << treeid.mirrors << "|" << MerklePointSerializer::toString(point);
	sendMessage(peer, msg.str());
}

void MessageSender::digestPing(const Peer& peer, const TreeId& treeid, const std::deque<MerklePoint>& points)
{
	std::stringstream msg;
	msg << "sync|tree=" << treeid.id << " n=" << treeid.mirrors << "|";
	msg << MerklePointSerializer::toString(points.front());
	for (auto it = ++points.begin(); it != points.end(); ++it)
		msg << "|" << MerklePointSerializer::toString(*it);
	sendMessage(peer, msg.str());
}

void MessageSender::requestKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last)
{
	std::stringstream msg;
	msg << "key-req|tree=" << treeid.id << " n=" << treeid.mirrors << " first=" << first << " last=" << last << "|";
	sendMessage(peer, msg.str());
}

void MessageSender::dropKey(const Peer& peer, const string& filename)
{
	sendMessage(peer, "drop|name=" + filename + "|");
}
