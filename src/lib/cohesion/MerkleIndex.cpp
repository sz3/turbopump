#include "MerkleIndex.h"

#include "common/MerklePoint.h"
#include "consistent_hashing/Hash.h"
#include <tuple>

void MerkleIndex::add(const std::string& id)
{
	// TODO: worry about endianness of keyhash.
	// if different architectures can't agree on where a file hashes to... we're in trouble.
	// right now, it's derived from a char*, so it's mostly ok...
	//  except that the merkle_point::keybits don't act how you might expect...
	unsigned long long keyhash = Hash::compute(id).integer();
	_tree.insert(keyhash, keyhash, id);
	//std::cout << "MerkleIndex::add " << id << ", " << keyhash << std::endl;
}

void MerkleIndex::remove(const std::string& id)
{
	unsigned long long keyhash = Hash::compute(id).integer();
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

// TODO: unsigned long long& start?
//       return "next"? ???
std::deque<std::string> MerkleIndex::enumerate(unsigned long long first, unsigned long long last) const
{
	static const unsigned limit = 50;
	std::deque<std::string> files;
	auto fun = [&] (unsigned long long hash, const std::string& file) { files.push_back(file); first = hash; return files.size() < limit; };

	_tree.enumerate(fun, first, last);
	return files;
}

std::ostream& operator<<(std::ostream& stream, const std::tuple<unsigned long long, std::string>& fileData)
{
	stream << std::get<1>(fileData);
	return stream;
}

void MerkleIndex::print(int keywidth) const
{
	_tree.print(keywidth);
}
