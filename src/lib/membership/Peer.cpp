#include "Peer.h"

#include "serialize/StringUtil.h"
#include <iostream>
using std::string;
using std::vector;

Peer::Peer(const string& uid)
	: uid(uid)
{
}

string Peer::address() const
{
	if (ips.empty())
		return "";
	return ips.front();
}

string Peer::toString() const
{
	string iplist = StringUtil::stlJoin(ips, '|');
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
