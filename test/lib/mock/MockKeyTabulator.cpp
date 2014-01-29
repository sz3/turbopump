/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockKeyTabulator.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"

void MockKeyTabulator::add(const std::string& key, unsigned mirrors)
{
	_history.call("add", key, mirrors);
}

void MockKeyTabulator::remove(const std::string& key, unsigned mirrors)
{
	_history.call("remove", key, mirrors);
}

void MockKeyTabulator::splitSection(const std::string& where)
{
	_history.call("splitSection", where);
}

void MockKeyTabulator::cannibalizeSection(const std::string& where)
{
	_history.call("cannibalizeSection", where);
}

const IDigestKeys& MockKeyTabulator::find(const std::string& id, unsigned mirrors) const
{
	_history.call("find", id, mirrors);
	return _tree;
}

const IDigestKeys& MockKeyTabulator::randomTree() const
{
	// determined by fair dice roll. Guaranteed to be random.
	_history.call("randomTree");
	return _tree;
}

const IDigestKeys& MockKeyTabulator::unwantedTree() const
{
	// determined by fair dice roll. Guaranteed to be random.
	_history.call("unwantedTree");
	return _tree;
}
