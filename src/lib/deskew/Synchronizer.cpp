/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Synchronizer.h"

#include "ICorrectSkew.h"
#include "IKeyTabulator.h"
#include "IDigestKeys.h"
#include "KeyRange.h"
#include "TreeId.h"

#include "common/MerklePoint.h"
#include "hashing/Hash.h"
#include "hashing/IConsistentHashRing.h"
#include "logging/ILog.h"
#include "membership/IKnowPeers.h"
#include "membership/Peer.h"
#include "peer_client/IMessageSender.h"

#include "util/random.h"
#include <deque>
#include <sstream>
using std::shared_ptr;
using std::string;

Synchronizer::Synchronizer(const IConsistentHashRing& ring, const IKnowPeers& membership, const IKeyTabulator& index, IMessageSender& messenger, ICorrectSkew& corrector, ILog& logger)
	: _ring(ring)
	, _membership(membership)
	, _index(index)
	, _messenger(messenger)
	, _corrector(corrector)
	, _logger(logger)
{
}

void Synchronizer::pingRandomPeer()
{
	const IDigestKeys& tree = _index.randomTree();
	std::vector<string> locations = _ring.locations(Hash().fromHash(tree.id().id), tree.id().mirrors);

	shared_ptr<Peer> peer = locations.empty()? _membership.randomPeer() : randomPeerFromList(locations);
	if (!peer)
		return;

	_logger.logTrace("pingRandomPeer to " + peer->uid);
	_messenger.digestPing(*peer, tree.id(), tree.top());
}

void Synchronizer::offloadUnwantedKeys()
{
	const IDigestKeys& tree = _index.unwantedTree();
	if (tree.empty())
		return;

	std::vector<string> locations = _ring.locations(Hash().fromHash(tree.id().id), tree.id().mirrors);
	if (locations.empty())
		return;

	shared_ptr<Peer> peer = randomPeerFromList(locations);
	if (!peer)
		return;
	// TODO: some sort of exchange will be necessary here.
	// "tell me if you have these" => "oh, you have A and B but not C?"
	// ... but for now, just chuck everything at them
	_corrector.pushKeyRange(*peer, tree.id(), 0, ~0ULL, _membership.self()->uid);
}

void Synchronizer::compare(const Peer& peer, const TreeId& treeid, const MerklePoint& point, bool isSyncResponse)
{
	_logger.logTrace("Synchronizer compare from " + peer.uid);

	// TODO: do we need to sanity check treeid against t4he _ring or _index to make sure we care?
	const IDigestKeys& tree = _index.find(treeid.id, treeid.mirrors);
	if (point == MerklePoint::null())
	{
		if (tree.empty())
			return;
		_corrector.pushKeyRange(peer, treeid, 0, ~0ULL);
		return;
	}

	std::deque<MerklePoint> diffs = tree.diff(point);

	/*std::stringstream ss;
	ss << " Synchronizer compare from " << peer.uid << " on tree " << treeid.id << ":" << treeid.mirrors << ". Point is " << MerklePointSerializer::toString(point) << ". Diffs are : ";
	for (auto it = diffs.begin(); it != diffs.end(); ++it)
		ss << MerklePointSerializer::toString(*it) << " , ";
	_logger.logDebug(ss.str());*/

	// 0 == no diff
	if (diffs.empty())
		return;

	/*_logger.logDebug("diff count for " + MerklePointSerializer::toString(point) + ": " + turbo::str::str(diffs.size()));
	for (const MerklePoint& diff : diffs)
		_logger.logDebug(MerklePointSerializer::toString(diff));*/
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
			// if keys are equal, we need to heal
			if (diff.location == point.location)
			{
				// try to push key, but also trigger complement check
				_messenger.requestKey(peer, treeid, diff.location.key);
				_corrector.pushKey(peer, treeid, diff.location.key);
			}
			else // we're missing a branch. This will try to send us one too many keys (the one already at diff.location) but the duplicate write will be rejected... so who cares?
			{
				KeyRange range(point.location);
				_messenger.requestKeyRange(peer, treeid, range.first(), range.last());
			}
		}
		else
		{
			KeyRange range(diff.location);
			_messenger.requestKeyRange(peer, treeid, range.first(), range.last());
		}
	}

	else //if (diffs.size() >= 2)
	{
		if (isSyncResponse)
			diffs.pop_back();

		// respond!
		_messenger.digestPing(peer, treeid, diffs);
	}
}

shared_ptr<Peer> Synchronizer::randomPeerFromList(std::vector<string> locs) const
{
	while (!locs.empty())
	{
		std::vector<string>::iterator it = turbo::random::select(locs);
		shared_ptr<Peer> peer = _membership.lookup(*it);
		if (!!peer)
			return peer;

		locs.erase(it);
	}
	return NULL;
}
