#include "Synchronizer.h"

#include "ICorrectSkew.h"
#include "IMerkleIndex.h"
#include "IMessageSender.h"

#include "common/MerklePoint.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include <deque>
#include <iostream>
using std::shared_ptr;

Synchronizer::Synchronizer(const IMembership& membership, const IMerkleIndex& index, IMessageSender& messenger, ICorrectSkew& corrector)
	: _membership(membership)
	, _index(index)
	, _messenger(messenger)
	, _corrector(corrector)
{
}

void Synchronizer::pingRandomPeer()
{
	shared_ptr<Peer> peer = _membership.randomPeer();
	if (!peer)
		return;
	_messenger.merklePing(*peer, _index.top());
}

void Synchronizer::compare(const Peer& peer, const MerklePoint& point)
{
	if (point == MerklePoint::null())
	{
		if (_index.top() == MerklePoint::null())
			return;
		_corrector.pushKeyRange(peer, 0, ~0);
		return;
	}

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
			_messenger.requestKeyRange(peer, diff.location.key, diff.location.key);
	}

	else if (diffs.size() >= 2)
	{
		// respond!
		_messenger.merklePing(peer, diffs);
	}
}
