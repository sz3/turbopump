#include "MessageSender.h"

#include "common/MerklePoint.h"
#include "wan_server/IPeerTracker.h"
#include "wan_server/PeerConnection.h"
#include "wan_server/ThrottledWriteStream.h"
#include <memory>
#include <sstream>
using std::shared_ptr;

MessageSender::MessageSender(IPeerTracker& peers)
	: _peers(peers)
{
}

bool MessageSender::sendMessage(const Peer& peer, const std::string& message)
{
	shared_ptr<PeerConnection> peerConn = _peers.track(peer);
	if (!peerConn)
		return false;

	ThrottledWriteStream writer(*peerConn);
	writer.write(message.data(), message.size());
	return true;
}

void MessageSender::merklePing(const Peer& peer, const MerklePoint& point)
{
	sendMessage(peer, "merkle||" + MerklePointSerializer::toString(point));
}

void MessageSender::merklePing(const Peer& peer, const std::deque<MerklePoint>& points)
{
	std::string message = "merkle||";
	message += MerklePointSerializer::toString(points.front());
	for (auto it = ++points.begin(); it != points.end(); ++it)
		message += "|" + MerklePointSerializer::toString(*it);
	sendMessage(peer, message);
}

void MessageSender::requestKeyRange(const Peer& peer, unsigned long long first, unsigned long long last)
{
	std::stringstream msg;
	msg << "key-req|first=" << first << " last=" << last << "|";
	sendMessage(peer, msg.str());
}
