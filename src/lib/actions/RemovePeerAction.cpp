/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "RemovePeerAction.h"

#include "deskew/IKeyTabulator.h"
#include "hashing/IConsistentHashRing.h"
#include "membership/IMembership.h"
using std::map;
using std::string;

RemovePeerAction::RemovePeerAction(IConsistentHashRing& ring, IMembership& membership, IKeyTabulator& keyTabulator)
	: _ring(ring)
	, _membership(membership)
	, _keyTabulator(keyTabulator)
{
}

std::string RemovePeerAction::name() const
{
	return "remove_peer";
}

bool RemovePeerAction::run(const DataBuffer& data)
{
	if (!_membership.remove(_uid))
		return true;
	if (!_membership.save())
		return false;
	_keyTabulator.cannibalizeSection(_uid);
	_ring.erase(_uid);
	return true;
}

void RemovePeerAction::setParams(const std::map<std::string,std::string>& params)
{
	map<string,string>::const_iterator it = params.find("uid");
	if (it != params.end())
		_uid = it->second;
}
