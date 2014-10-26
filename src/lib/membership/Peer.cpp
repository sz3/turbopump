/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Peer.h"

#include "serialize/StringUtil.h"
#include "socket/socket_address.h"
#include <atomic>
#include <iostream>
using std::string;
using std::vector;

Peer::Peer(const string& uid)
	: uid(uid)
	, _actionId(0)
{
}

Peer::Peer(const Peer& peer)
	: uid(peer.uid)
	, ips(peer.ips)
	, _actionId(0)
{
}

std::string Peer::address() const
{
	if (ips.empty())
		return "";
	return ips.front();
}

// does atomic_increment on in-memory value
unsigned char Peer::nextActionId() const
{
	return _actionId++;
}

string Peer::toString() const
{
	string iplist = StringUtil::join(ips, '|');
	if (iplist.empty())
		iplist = "none";
	return uid + " " + iplist;
}

bool Peer::fromString(const string& serial)
{
	vector<string> splits = StringUtil::split(serial, ' ');
	if (splits.size() < 2)
		return false;

	uid = splits[0];
	if (splits[1] != "none")
		ips = StringUtil::split(splits[1], '|');
	return true;
}
