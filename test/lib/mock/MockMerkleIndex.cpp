#include "MockMerkleIndex.h"

#include "cohesion/MerklePoint.h"
#include "membership/Peer.h"

void MockMerkleIndex::add(const std::string& id)
{
	_history.call("add", id);
}

void MockMerkleIndex::remove(const std::string& id)
{
	_history.call("remove", id);
}

std::deque<MerklePoint> MockMerkleIndex::diff(const MerklePoint& point) const
{
	_history.call("diff", MerklePointSerializer::toString(point));
	return _diff;
}
