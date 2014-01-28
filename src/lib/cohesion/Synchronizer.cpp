/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Synchronizer.h"

#include "ICorrectSkew.h"
#include "IMerkleIndex.h"
#include "IMerkleTree.h"
#include "KeyRange.h"

#include "actions_req/IMessageSender.h"
#include "cohesion/TreeId.h"
#include "common/MerklePoint.h"
#include "consistent_hashing/IHashRing.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include <deque>
#include <iostream>
using std::shared_ptr;
using std::string;

Synchronizer::Synchronizer(const IHashRing& ring, const IMembership& membership, const IMerkleIndex& index, IMessageSender& messenger, ICorrectSkew& corrector)
	: _ring(ring)
	, _membership(membership)
	, _index(index)
	, _messenger(messenger)
	, _corrector(corrector)
{
}

void Synchronizer::pingRandomPeer()
{
	const IMerkleTree& tree = _index.randomTree();
	std::vector<string> locations = _ring.locationsFromHash(tree.id().id, tree.id().mirrors);

	shared_ptr<Peer> peer = locations.empty()? _membership.randomPeer() : _membership.randomPeerFromList(locations);
	if (!peer)
		return;
	_messenger.merklePing(*peer, tree.id(), tree.top());
}

void Synchronizer::offloadUnwantedKeys()
{
	const IMerkleTree& tree = _index.unwantedTree();
	if (tree.empty())
		return;

	std::vector<string> locations = _ring.locationsFromHash(tree.id().id, tree.id().mirrors);
	if (locations.empty())
		return;

	shared_ptr<Peer> peer = _membership.randomPeerFromList(locations);
	if (!peer)
		return;
	// TODO: some sort of exchange will be necessary here.
	// "tell me if you have these" => "oh, you have A and B but not C?"
	// ... but for now, just chuck everything at them
	_corrector.pushKeyRange(*peer, tree.id(), 0, ~0ULL, _membership.self()->uid);
}

void Synchronizer::compare(const Peer& peer, const TreeId& treeid, const MerklePoint& point)
{
	// TODO: do we need to sanity check treeid against the _ring or _index to make sure we care?
	const IMerkleTree& tree = _index.find(treeid.id, treeid.mirrors);
	if (point == MerklePoint::null())
	{
		if (tree.empty())
			return;
		pushKeyRange(peer, treeid, 0, ~0ULL);
		return;
	}

	std::deque<MerklePoint> diffs = tree.diff(point);
	/*std::cout << " Synchronizer compare. Point is " << MerklePointSerializer::toString(point) << ". Diffs are : ";
	for (auto it = diffs.begin(); it != diffs.end(); ++it)
		std::cout << MerklePointSerializer::toString(*it) << " , ";
	std::cout << std::endl;*/

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
			_messenger.requestKeyRange(peer, treeid, 0, ~0ULL);
		else if (diff.location.keybits == 64)
		{
			//if (diff.location.key == point.location.key)
			//	_corrector.healKey(peer, diff.location.key);

			KeyRange range(point.location);
			_messenger.requestKeyRange(peer, treeid, range.first(), range.last());
		}
		else
		{
			KeyRange range(diff.location);
			_messenger.requestKeyRange(peer, treeid, range.first(), range.last());
		}
	}

	else if (diffs.size() >= 2)
	{
		// respond!
		_messenger.merklePing(peer, treeid, diffs);
	}
}

void Synchronizer::pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last)
{
	_corrector.pushKeyRange(peer, treeid, first, last);
}
