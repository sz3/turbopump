/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockMessageSender.h"

#include "common/MerklePoint.h"
#include "conformity/TreeId.h"
#include "membership/Peer.h"

void MockMessageSender::digestPing(const Peer& peer, const TreeId& treeid, const MerklePoint& point)
{
	_history.call("digestPing", peer.uid, treeid.id, MerklePointSerializer::toString(point));
}

void MockMessageSender::digestPing(const Peer& peer, const TreeId& treeid, const std::deque<MerklePoint>& points)
{
	std::string pstr;
	if (!points.empty())
		pstr += MerklePointSerializer::toString(points.front());
	for (auto it = ++points.begin(); it != points.end(); ++it)
		pstr += "|" + MerklePointSerializer::toString(*it);
	_history.call("digestPing", peer.uid, treeid.id, pstr);
}

void MockMessageSender::requestKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last)
{
	_history.call("requestKeyRange", peer.uid, treeid.id, first, last);
}

void MockMessageSender::requestHealKey(const Peer& peer, const TreeId& treeid, unsigned long long key)
{
	_history.call("requestHealKey", peer.uid, treeid.id, key);
}

void MockMessageSender::offerWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source)
{
	_history.call("offerWrite", peer.uid, filename, version, source);
}

void MockMessageSender::demandWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source)
{
	_history.call("demandWrite", peer.uid, filename, version, source);
}

void MockMessageSender::acknowledgeWrite(const Peer& peer, const std::string& filename, const std::string& version, unsigned long long size)
{
	_history.call("acknowledgeWrite", peer.uid, filename, version, size);
}
