/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ModifyPeer.h"

#include "api/WriteInstructions.h"
#include "common/VectorClock.h"
#include "common/turbopump_defaults.h"
#include "deskew/IKeyTabulator.h"
#include "hashing/IConsistentHashRing.h"
#include "membership/IKnowPeers.h"
#include "membership/Peer.h"
#include "storage/readstream.h"

#include "socket/StringByteStream.h"
#include <vector>
using std::string;
using std::vector;

ModifyPeer::ModifyPeer(IConsistentHashRing& ring, IKnowPeers& membership, IKeyTabulator& keyTabulator)
	: _ring(ring)
	, _membership(membership)
	, _keyTabulator(keyTabulator)
{
}

bool ModifyPeer::run(WriteInstructions& params, readstream& contents)
{
	if (params.name.find(MEMBERSHIP_FILE_PREFIX) != 0)
		return false;
	string uid = params.name.substr(MEMBERSHIP_FILE_PREFIX_LENGTH-1);

	VectorClock version;
	version.fromString(params.version);
	if (version.isDeleted())
		return remove(uid);
	else
		return update(uid, contents);
}

bool ModifyPeer::update(const std::string& uid, readstream& contents)
{
	StringByteStream stream;
	contents.stream(stream);
	string ip = stream.writeBuffer();

	vector<string> ips = {ip};
	bool isNew = _membership.update(uid, ips);
	if (!_membership.save())
		return false;
	if (!isNew)
		return true;

	_ring.insert(uid, uid);
	_keyTabulator.splitSection(uid);
	return true;
}

bool ModifyPeer::remove(const std::string& uid)
{
	if (!_membership.remove(uid) or !_membership.save())
		return false;

	_ring.erase(uid);
	_keyTabulator.cannibalizeSection(uid);
	return true;
}
