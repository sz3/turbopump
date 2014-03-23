/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockSynchronize.h"

#include "cohesion/TreeId.h"
#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockSynchronize::compare(const Peer& peer, const TreeId& tree, const MerklePoint& point, bool isSyncResponse)
{
	_history.call("compare", peer.uid, tree.id, tree.mirrors, MerklePointSerializer::toString(point), isSyncResponse);
}

void MockSynchronize::pushKeyRange(const Peer& peer, const TreeId& tree, unsigned long long first, unsigned long long last)
{
	_history.call("pushKeyRange", peer.uid, tree.id, tree.mirrors, first, last);
}
