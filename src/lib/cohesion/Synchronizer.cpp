#include "Synchronizer.h"

#include "ICorrectSkew.h"
#include "IMerkleIndex.h"

#include "actions_req/IMessageSender.h"
#include "common/MerklePoint.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include <deque>
#include <endian.h>
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
		// 1 = leaf disagreement Need to do proper "healing" operation comparing two files to figure out how to fix it.
		if (diff.location.keybits == point.location.keybits)
			_corrector.healKey(peer, diff.location.key);
		else
		{
			// TODO: maybe move this logic into a utility class?
			// merkle_tree imposes big endianness on its keys (because it treats them as char*s)
			// which means any and all fancy maths on MerklePoint needs to keep this big endianness in mind.
			unsigned shift = (sizeof(diff.location.key)<<3) - diff.location.keybits;
			unsigned long long first = htobe64(diff.location.key);
			first = (first >> shift) << shift;
			unsigned long long last = first xor (~0ULL >> diff.location.keybits);

			_messenger.requestKeyRange(peer, be64toh(first), be64toh(last));
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
