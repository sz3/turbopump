/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Synchronizer.h"

#include "ICorrectSkew.h"
#include "IKeyTabulator.h"
#include "IDigestKeys.h"
#include "KeyRange.h"
#include "TreeId.h"

#include "actions_req/IMessageSender.h"
#include "common/MerklePoint.h"
#include "consistent_hashing/Hash.h"
#include "consistent_hashing/IConsistentHashRing.h"
#include "logging/ILog.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include <deque>
#include <sstream>
using std::shared_ptr;
using std::string;

Synchronizer::Synchronizer(const IConsistentHashRing& ring, const IMembership& membership, const IKeyTabulator& index, IMessageSender& messenger, ICorrectSkew& corrector, ILog& logger)
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
	std::vector<string> locations = _ring.locations(Hash().fromBase64(tree.id().id), tree.id().mirrors);

	shared_ptr<Peer> peer = locations.empty()? _membership.randomPeer() : _membership.randomPeerFromList(locations);
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

	std::vector<string> locations = _ring.locations(Hash().fromBase64(tree.id().id), tree.id().mirrors);
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
			if (diff.location.key == point.location.key)
			{
				// send healKey packet to trigger complement check
				_messenger.requestHealKey(peer, treeid, diff.location.key);
				_corrector.healKey(peer, treeid, diff.location.key);
			}
			else // if keys are not, we don't have the branch
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
