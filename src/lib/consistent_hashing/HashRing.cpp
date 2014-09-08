/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "HashRing.h"

#include "Hash.h"
using std::string;
using std::vector;

string HashRing::hash(const string& str)
{
	return Hash(str).base64();
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

vector<string> HashRing::locations(const string& filename, unsigned numLocs) const
{
	return locationsFromHash(hash(filename), numLocs);
}

vector<string> HashRing::locationsFromHash(const string& hash, unsigned numLocs) const
{
	vector<string> locs;
	lookupFromHash(hash, locs, numLocs);
	return locs;
}

string HashRing::section(const string& filename) const
{
	vector<string> locs;
	return lookup(filename, locs, 0);
}

string HashRing::lookup(const string& filename, vector<string>& locs, unsigned numLocs) const
{
	return lookupFromHash(hash(filename), locs, numLocs);
}

string HashRing::lookupFromHash(const string& hash, vector<string>& locs, unsigned numLocs) const
{
	circular_map<string,string>::iterator it = _ring.lower_bound(hash);
	if (it == _ring.end())
		return "";

	string section = it->first;
	unsigned count = 0;
	while (!it.bad() && count < numLocs)
	{
		locs.push_back(it->second);
		++it;
		++count;
	}
	return section;
}

unsigned HashRing::size() const
{
	return _ring.size();
}

std::string HashRing::toString() const
{
	std::string result;
	for (circular_map<string,string>::iterator it = _ring.begin(); it != _ring.end(); ++it)
		result += it->first + ":" + it->second + " ";
	return result;
}
