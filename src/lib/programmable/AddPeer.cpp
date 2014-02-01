/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "AddPeer.h"

#include "actions/WriteParams.h"
#include "cohesion/IKeyTabulator.h"
#include "common/turbopump_defaults.h"
#include "consistent_hashing/IHashRing.h"
#include "data_store/DataEntry.h"
#include "membership/IMembership.h"
using std::string;

AddPeer::AddPeer(IHashRing& ring, IMembership& membership, IKeyTabulator& keyTabulator)
	: _ring(ring)
	, _membership(membership)
	, _keyTabulator(keyTabulator)
{
}

bool AddPeer::run(WriteParams params, IDataStoreReader::ptr contents)
{
	if (params.filename.find(MEMBERSHIP_FILE_PREFIX) == string::npos)
		return false;
	string uid = params.filename.substr(MEMBERSHIP_FILE_PREFIX_LENGTH-1);
	string ip = contents->data().data;

	bool isNew = _membership.add(uid);
	_membership.addIp(ip, uid);
	if (!_membership.save())
		return false;
	if (!isNew)
		return true;

	_ring.addWorker(uid);
	_keyTabulator.splitSection(uid);
	return true;
}
