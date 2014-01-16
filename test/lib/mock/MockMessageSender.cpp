/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockMessageSender.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockMessageSender::merklePing(const Peer& peer, const std::string& treeid, const MerklePoint& point)
{
	_history.call("merklePing", peer.uid, treeid, MerklePointSerializer::toString(point));
}

void MockMessageSender::merklePing(const Peer& peer, const std::string& treeid, const std::deque<MerklePoint>& points)
{
	std::string pstr;
	if (!points.empty())
		pstr += MerklePointSerializer::toString(points.front());
	for (auto it = ++points.begin(); it != points.end(); ++it)
		pstr += "|" + MerklePointSerializer::toString(*it);
	_history.call("merklePing", peer.uid, treeid, pstr);
}

void MockMessageSender::requestKeyRange(const Peer& peer, const std::string& treeid, unsigned long long first, unsigned long long last)
{
	_history.call("requestKeyRange", peer.uid, treeid, first, last);
}

void MockMessageSender::dropKey(const Peer& peer, const std::string& filename)
{
	_history.call("dropKey", peer.uid, filename);
}
