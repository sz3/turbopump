#include "MockSynchronize.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockSynchronize::compare(const Peer& peer, const MerklePoint& point)
{
	_history.call("compare", peer.toString(), MerklePointSerializer::toString(point));
}
