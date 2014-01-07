/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "HashRing.h"

#include "base64.h"
#include "Hash.h"
using std::vector;

string HashRing::hash(const string& str)
{
	std::string result(Hash::compute(str).bytes());
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
