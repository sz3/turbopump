#include "MockMessageSender.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockMessageSender::merklePing(const Peer& peer, const MerklePoint& point)
{
	_history.call("merklePing", peer.uid, MerklePointSerializer::toString(point));
}

void MockMessageSender::merklePing(const Peer& peer, const std::deque<MerklePoint>& points)
{
	std::string pstr;
	if (!points.empty())
		pstr += MerklePointSerializer::toString(points.front());
	for (auto it = ++points.begin(); it != points.end(); ++it)
		pstr += "|" + MerklePointSerializer::toString(*it);
	_history.call("merklePing", peer.uid, pstr);
}

void MockMessageSender::requestKeyRange(const Peer& peer, unsigned long long first, unsigned long long last)
{
	_history.call("requestKeyRange", peer.uid, first, last);
}
