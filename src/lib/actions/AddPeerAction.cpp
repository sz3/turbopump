/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "AddPeerAction.h"

#include "cohesion/IMerkleIndex.h"
#include "consistent_hashing/IHashRing.h"
#include "membership/IMembership.h"
using std::map;
using std::string;

AddPeerAction::AddPeerAction(IHashRing& ring, IMembership& membership, IMerkleIndex& merkleIndex)
	: _ring(ring)
	, _membership(membership)
	, _merkleIndex(merkleIndex)
{
}

std::string AddPeerAction::name() const
{
	return "add-peer";
}

bool AddPeerAction::run(const DataBuffer& data)
{
	bool isNew = _membership.add(_uid);
	_membership.addIp(_ip, _uid);
	if (!_membership.save())
		return false;
	if (!isNew)
		return true;

	_ring.addWorker(_uid);
	_merkleIndex.splitTree(_uid);
	return true;
}

void AddPeerAction::setParams(const std::map<std::string,std::string>& params)
{
	map<string,string>::const_iterator it = params.find("uid");
	if (it != params.end())
		_uid = it->second;

	it = params.find("ip");
	if (it != params.end())
		_ip = it->second;
}
