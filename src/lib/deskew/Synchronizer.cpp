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

	MerkleDiffResult diffs = tree.diff(point);

	/*std::stringstream ss;
	ss << " Synchronizer compare from " << peer.uid << " on tree " << treeid.id << ":" << treeid.mirrors << ". Point is " << MerklePointSerializer::toString(point) << ". Diffs are : ";
	for (auto it = diffs.begin(); it != diffs.end(); ++it)
		ss << MerklePointSerializer::toString(*it) << " , ";
	_logger.logDebug(ss.str());*/

	if (diffs.no_difference())
		return;

	else if (diffs.traverse())
	{
		if (isSyncResponse)
			diffs.points().pop_back();

		// respond!
		_messenger.digestPing(peer, treeid, diffs.points());
	}

	else if (diffs.need_range())
	{
		KeyRange range(point);
		_messenger.requestKeyRange(peer, treeid, range.first(), range.last());
	}

	else if (diffs.need_partial_range())
	{
		// this is confusing, but the gist is that we want to narrow down the range we request from
		// the other side. If the "inner" range is outside the bounds of the "outer" range or they are equal,
		// the best we can do is ask for the entire outer range.
		// but if the inner range is actually a subset of the outer range, we can focus on the ranges
		// that the inner range (that's us!) doesn't have any information on.
		KeyRange outer(point);
		KeyRange inner(diffs[0]);
		if (outer.first() > inner.first() || inner.first() > outer.last() || (outer.first() == inner.first() && outer.last() == inner.last()))
			_messenger.requestKeyRange(peer, treeid, outer.first(), outer.last());
		else
		{
			if (outer.first() < inner.first())
				_messenger.requestKeyRange(peer, treeid, outer.first(), inner.first());
			if (inner.last() < outer.last())
				_messenger.requestKeyRange(peer, treeid, inner.last(), outer.last());
		}

	}

	else if (diffs.need_exchange())
	{
		// try to push key, but also trigger complement check
		const MerklePoint& diff = diffs[0];
		_messenger.requestKey(peer, treeid, diff.key);
		_corrector.pushKey(peer, treeid, diff.key);
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
