/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "AddPeer.h"

#include "cohesion/IKeyTabulator.h"
#include "common/KeyMetadata.h"
#include "common/turbopump_defaults.h"
#include "consistent_hashing/IHashRing.h"
#include "data_store/DataEntry.h"
#include "membership/IMembership.h"
using std::string;

AddPeer::AddPeer(IHashRing& ring, IMembership& membership, IKeyTabulator& merkleIndex)
	: _ring(ring)
	, _membership(membership)
	, _merkleIndex(merkleIndex)
{
}

bool AddPeer::run(KeyMetadata md, IDataStoreReader::ptr contents)
{
	if (md.filename.find(MEMBERSHIP_FILE_PREFIX) == string::npos)
		return false;
	string uid = md.filename.substr(MEMBERSHIP_FILE_PREFIX_LENGTH-1);
	string ip = contents->data().data;

	bool isNew = _membership.add(uid);
	_membership.addIp(ip, uid);
	if (!_membership.save())
		return false;
	if (!isNew)
		return true;

	_ring.addWorker(uid);
	_merkleIndex.splitSection(uid);
	return true;
}
