#pragma once

/*
something that looks an awful lot like a Radix tree.
N-bit keys (likely 128-bit truncated tiger) -> 1-64 keys corresponding to children.
	each child is either another tree or a single hash value (if a leaf)

Levels of the tree are serialized with a simple enumeration of the current node values.

Levels of the tree are stored as an array (std::vector) of a { hash, ptr } type. If the node is a leaf, ptr is null. For size concerns, could experiment with using a union instead, doing union { hash, ptr }, utilizing the first bit (making it a 127-bit hash, for example) as the switch between the two. This would effect cache locality, and there may be a tradeoff between speed and memory footprint.

Future optimizations include:
	Grow the vector to 64 keys only as needed, not at the start. (ram)
	
*/

#include "IMerkleIndex.h"
#include "data_structures/merkle_tree.h"

class MerkleIndex : public IMerkleIndex
{
public:
	// key is filename hash
	// value really ought to be hash of the file contents, right? What about versioning?
	void add(const std::string& id);
	void remove(const std::string& id);

	MerklePoint top() const;
	std::deque<MerklePoint> diff(const MerklePoint& point) const;

protected:
	merkle_tree<unsigned long long, unsigned long long, std::string> _tree;
};
