/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockMembership.h"

#include "membership/Peer.h"

MockMembership::MockMembership()
	: _self( new Peer("me") )
{
}

bool MockMembership::save()
{
	_history.call("save");
	return true;
}

bool MockMembership::add(const std::string& uid)
{
	_history.call("add", uid);
	if (_ips[uid])
		return false;

	_ips[uid].reset(new Peer(uid));
	return true;
}

bool MockMembership::addIp(const std::string& ip, const std::string& uid)
{
	_history.call("addIp", ip, uid);
	_ips[ip].reset(new Peer(uid));
	return true;
}

bool MockMembership::remove(const std::string& uid)
{
	_history.call("remove", uid);
	return _ips.erase(uid);
}

std::shared_ptr<Peer> MockMembership::lookup(const std::string& uid) const
{
	_history.call("lookup", uid);
	auto it = _ips.find(uid);
	return it != _ips.end()? it->second : NULL;
}

std::shared_ptr<Peer> MockMembership::lookupIp(const std::string& ip) const
{
	_history.call("lookupIp", ip);
	auto it = _ips.find(ip);
	return it != _ips.end()? it->second : NULL;
}

std::shared_ptr<Peer> MockMembership::self() const
{
	_history.call("self");
	return _self;
}

std::shared_ptr<Peer> MockMembership::randomPeer() const
{
	_history.call("randomPeer");
	return _ips.empty()? NULL : _ips.begin()->second;
}

std::shared_ptr<Peer> MockMembership::randomPeerFromList(std::vector<std::string> list) const
{
	_history.call("randomPeerFromList");
	return _ips.empty()? NULL : _ips.begin()->second;
}

std::string MockMembership::toString() const
{
	return "mock membership";
}
