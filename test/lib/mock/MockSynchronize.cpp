/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockSynchronize.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockSynchronize::compare(const Peer& peer, const std::string& treeid, const MerklePoint& point)
{
	_history.call("compare", peer.uid, treeid, MerklePointSerializer::toString(point));
}

void MockSynchronize::pushKeyRange(const Peer& peer, const std::string& treeid, unsigned long long first, unsigned long long last)
{
	_history.call("pushKeyRange", peer.uid, treeid, first, last);
}
