#include "MockSynchronize.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockSynchronize::compare(const Peer& peer, const MerklePoint& point)
{
	_history.call("compare", peer.uid, MerklePointSerializer::toString(point));
}

void MockSynchronize::pushKeyRange(const Peer& peer, unsigned long long first, unsigned long long last)
{
	_history.call("pushKeyRange", peer.uid, first, last);
}
