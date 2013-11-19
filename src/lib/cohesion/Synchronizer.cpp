#include "Synchronizer.h"

#include "ICorrectSkew.h"
#include "IMerkleIndex.h"

#include "common/DataBuffer.h"
#include "common/MerklePoint.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include "wan_server/IPeerTracker.h"
#include "wan_server/PeerConnection.h"
#include "wan_server/ThrottledWriteStream.h"
#include <deque>
#include <iostream>
using std::shared_ptr;

Synchronizer::Synchronizer(const IMembership& membership, IPeerTracker& peers, const IMerkleIndex& index, ICorrectSkew& corrector)
	: _membership(membership)
	, _peers(peers)
	, _index(index)
	, _corrector(corrector)
{
}

bool Synchronizer::sendMessage(const Peer& peer, const std::string& message)
{
	shared_ptr<PeerConnection> peerConn = _peers.track(peer);
	if (!peerConn)
		return false;

	ThrottledWriteStream writer(*peerConn);
	writer.write(message.data(), message.size());
	return true;
}

void Synchronizer::pingRandomPeer()
{
	shared_ptr<Peer> peer = _membership.randomPeer();
	if (!peer)
		return;

	sendMessage(*peer, "ping||");
}

void Synchronizer::compare(const Peer& peer, const MerklePoint& point)
{
	std::deque<MerklePoint> diffs = _index.diff(point);

	// 0 == no diff
	if (diffs.empty())
		return;

	if (diffs.size() == 1)
	{
		MerklePoint& diff = diffs.front();
		// 1 = leaf disagreement Need to do proper "healing" operation comparing two files to figure out how to fix it.
		if (diff.location.keybits == point.location.keybits)
			_corrector.healKey(peer, diff.location.key);
		else
			_corrector.requestKeyRange(peer, diff.location.key, diff.location.key);
	}

	else if (diffs.size() >= 2)
	{
		// respond!
		std::string message = "merkle||";
		message += MerklePointSerializer::toString(diffs.front());
		for (auto it = ++diffs.begin(); it != diffs.end(); ++it)
			message += "|" + MerklePointSerializer::toString(*it);
		sendMessage(peer, message);
	}
}
