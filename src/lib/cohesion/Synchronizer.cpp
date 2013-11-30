#include "Synchronizer.h"

#include "ICorrectSkew.h"
#include "IMerkleIndex.h"
#include "MerkleRange.h"

#include "actions_req/IMessageSender.h"
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
		pushKeyRange(peer, 0, ~0ULL);
		return;
	}

	std::deque<MerklePoint> diffs = _index.diff(point);

	// 0 == no diff
	if (diffs.empty())
		return;

	if (diffs.size() == 1)
	{
		MerklePoint& diff = diffs.front();
		// TODO: address case for multiple workers having same key
		MerkleRange range(diff.location);
		_messenger.requestKeyRange(peer, range.first(), range.last());
	}

	else if (diffs.size() >= 2)
	{
		// respond!
		_messenger.merklePing(peer, diffs);
	}
}

void Synchronizer::pushKeyRange(const Peer& peer, unsigned long long first, unsigned long long last)
{
	_corrector.pushKeyRange(peer, first, last);
}
