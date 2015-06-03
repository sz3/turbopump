/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "AddPeer.h"

#include "api/WriteInstructions.h"
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

AddPeer::AddPeer(IConsistentHashRing& ring, IKnowPeers& membership, IKeyTabulator& keyTabulator)
	: _ring(ring)
	, _membership(membership)
	, _keyTabulator(keyTabulator)
{
}

bool AddPeer::run(WriteInstructions& params, readstream& contents)
{
	if (params.name.find(MEMBERSHIP_FILE_PREFIX) != 0)
		return false;
	string uid = params.name.substr(MEMBERSHIP_FILE_PREFIX_LENGTH-1);

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
