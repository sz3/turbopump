#include "HashRing.h"

#include "base64.h"
#include <cryptopp/tiger.h>
using std::vector;

string HashRing::hash(const string& str)
{
	std::string result;
	result.resize(CryptoPP::Tiger::DIGESTSIZE);
	CryptoPP::Tiger().CalculateDigest((unsigned char*)(&result[0]), (unsigned char*)(&str[0]), str.size());

	return base64_encode((const unsigned char*)result.data(), result.size());
}

// TODO: rather than updating the structure directly, it may be beneficial to implement a copy-on-write strategy to handle changes.
//  1) there shouldn't be a tremendous amount of changes. It's probably ok if they're expensive.
//  2) the ring should be small enough to do a copy-on-write operation.
//  3) mutex will only need to be obtained on write, rather than always. (currently: not at all)
void HashRing::addWorker(const string& name)
{
	// eventually, hold a merkle tree ptr?
	// also, implement insert(pair<>)
	_ring[hash(name)] = name;
}

void HashRing::removeWorker(const string& name)
{
	_ring.erase(hash(name));
}

void HashRing::growWorker(const string& name)
{
	// TODO
}

void HashRing::shrinkWorker(const string& name)
{
	// TODO
}

std::vector<string> HashRing::lookup(const string& filename, unsigned numWorkers) const
{
	vector<string> workers;
	circular_map<string,string>::iterator it = _ring.lower_bound(hash(filename));
	if (it == _ring.end())
		return workers;

	unsigned count = 0;
	while (!it.bad() && count < numWorkers)
	{
		workers.push_back(it->second);
		++it;
		++count;
	}
	return workers;
}

unsigned HashRing::size() const
{
	return _ring.size();
}
