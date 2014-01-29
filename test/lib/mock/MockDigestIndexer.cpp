/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockDigestIndexer.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockDigestIndexer::add(const std::string& key)
{
	_history.call("add", key);
}

void MockDigestIndexer::remove(const std::string& key)
{
	_history.call("remove", key);
}

void MockDigestIndexer::splitSection(const std::string& where)
{
	_history.call("splitSection", where);
}

void MockDigestIndexer::cannibalizeSection(const std::string& where)
{
	_history.call("cannibalizeSection", where);
}

const IDigestKeys& MockDigestIndexer::find(const std::string& id) const
{
	_history.call("find", id);
	return _tree;
}

const IDigestKeys& MockDigestIndexer::randomTree() const
{
	// determined by fair dice roll. Guaranteed to be random.
	_history.call("randomTree");
	return _tree;
}

const IDigestKeys& MockDigestIndexer::unwantedTree() const
{
	// determined by fair dice roll. Guaranteed to be random.
	_history.call("unwantedTree");
	return _tree;
}
