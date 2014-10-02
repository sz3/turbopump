/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "AddPeer.h"

#include "api/WriteInstructions.h"
#include "common/turbopump_defaults.h"
#include "deskew/IKeyTabulator.h"
#include "hashing/IConsistentHashRing.h"
#include "membership/IMembership.h"
#include "socket/StringByteStream.h"
using std::string;

AddPeer::AddPeer(IConsistentHashRing& ring, IMembership& membership, IKeyTabulator& keyTabulator)
	: _ring(ring)
	, _membership(membership)
	, _keyTabulator(keyTabulator)
{
}

bool AddPeer::run(WriteInstructions& params, IDataStoreReader::ptr contents)
{
	if (params.name.find(MEMBERSHIP_FILE_PREFIX) == string::npos)
		return false;
	string uid = params.name.substr(MEMBERSHIP_FILE_PREFIX_LENGTH-1);

	StringByteStream stream;
	contents->read(stream);
	string ip = stream.writeBuffer();

	bool isNew = _membership.add(uid);
	_membership.addIp(ip, uid);
	if (!_membership.save())
		return false;
	if (!isNew)
		return true;

	_ring.insert(uid, uid);
	_keyTabulator.splitSection(uid);
	return true;
}
