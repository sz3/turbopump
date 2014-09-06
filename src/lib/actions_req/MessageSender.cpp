/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MessageSender.h"

#include "common/MerklePoint.h"
#include "deskew/TreeId.h"
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

bool MessageSender::sendMessage(const Peer& peer, const string& message, bool blocking)
{
	shared_ptr<IBufferedConnectionWriter> writer(_peers.getWriter(peer));
	if (!writer)
		return false;

	writer->write(peer.nextActionId(), message.data(), message.size(), blocking);
	writer->flush(blocking);
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

void MessageSender::requestHealKey(const Peer& peer, const TreeId& treeid, unsigned long long key)
{
	std::stringstream msg;
	msg << "heal-key|tree=" << treeid.id << " n=" << treeid.mirrors << " key=" << key << "|";
	sendMessage(peer, msg.str());
}

void MessageSender::offerWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source)
{
	sendMessage(peer, "offer-write|name=" + filename + " v=" + version + " source=" + source + "|");
}

void MessageSender::demandWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source)
{
	sendMessage(peer, "demand-write|name=" + filename + " v=" + version + " source=" + source + "|");
}

// TODO: since this could block, may need to move write acks to their own thread.
// or do a two stage "try -> fail -> schedule retry" thing.
void MessageSender::acknowledgeWrite(const Peer& peer, const string& filename, const std::string& version, unsigned long long size)
{
	std::stringstream msg;
	msg << "ack-write|name=" << filename << " v=" << version << " size=" << size << "|";
	sendMessage(peer, msg.str(), true);
}
