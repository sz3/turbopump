/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockSynchronize.h"

#include "common/MerklePoint.h"
#include "deskew/TreeId.h"
#include "membership/Peer.h"

void MockSynchronize::compare(const Peer& peer, const TreeId& tree, const MerklePoint& point, bool isSyncResponse)
{
	_history.call("compare", peer.uid, tree.id, tree.mirrors, MerklePointSerializer::toString(point), isSyncResponse);
}
