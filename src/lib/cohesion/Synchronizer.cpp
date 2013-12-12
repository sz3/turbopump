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
	std::cout << " Synchronizer compare. Point is " << MerklePointSerializer::toString(point) << ". Diffs are : ";
	for (auto it = diffs.begin(); it != diffs.end(); ++it)
		std::cout << MerklePointSerializer::toString(*it) << " , ";
	std::cout << std::endl;

	// 0 == no diff
	if (diffs.empty())
		return;

	if (diffs.size() == 1)
	{
		MerklePoint& diff = diffs.front();
		/*
		 * TODO:
		 *  what we know here is that we did a diff and got a mismatched leaf back. It might
		 *  1) be a missing branch (empty hash), matching the keybits in point.location. This is the standard requestKeyRange case.
		 *  OR
		 *  2) be a leaf node, meaning returned keybits is maxkeybits
		 *     a) we have the nearest match -- i.e, point location doesn't match
		 *     OR
		 *     b) we have an incomplete range because there are many keys and we only have the one
		 *     OR
		 *     c) we have conflicting values for the same key
		 */


		if (diff == MerklePoint::null())
			_messenger.requestKeyRange(peer, 0, ~0ULL);
		else if (diff.location.keybits == 64)
		{
			//if (diff.location.key == point.location.key)
			//	_corrector.healKey(peer, diff.location.key);

			MerkleRange range(point.location);
			_messenger.requestKeyRange(peer, range.first(), range.last());
		}
		else
		{
			MerkleRange range(diff.location);
			_messenger.requestKeyRange(peer, range.first(), range.last());
		}
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
