#include "MerkleIndex.h"

#include "common/MerklePoint.h"

void MerkleIndex::add(const std::string& id)
{
	unsigned long long keyhash = *(unsigned long long*)id.data();
	_tree.insert(keyhash, keyhash);
}

void MerkleIndex::remove(const std::string& id)
{
	unsigned long long keyhash = *(unsigned long long*)id.data();
	_tree.remove(keyhash);
}

MerklePoint MerkleIndex::top() const
{
	return _tree.top();
}

std::deque<MerklePoint> MerkleIndex::diff(const MerklePoint& point) const
{
	return _tree.diff(point.location, point.hash);
}
