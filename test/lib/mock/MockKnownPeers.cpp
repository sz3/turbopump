/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockKnownPeers.h"

#include "serialize/str_join.h"
#include "socket/socket_address.h"

MockKnownPeers::MockKnownPeers()
	: _self( new Peer("me") )
{
}

bool MockKnownPeers::update(const std::string& uid, const std::vector<std::string>& ips)
{
	_history.call("update", uid, turbo::str::join(ips));
	if (_ips[uid])
		return false;

	_ips[uid].reset(new Peer(uid, ips));
	if (!ips.empty())
		_ips[ips.front()] = _ips[uid];
	return true;
}

bool MockKnownPeers::remove(const std::string& uid)
{
	_history.call("remove", uid);
	return _ips.erase(uid);
}

std::shared_ptr<Peer> MockKnownPeers::self() const
{
	_history.call("self");
	return _self;
}

std::shared_ptr<Peer> MockKnownPeers::lookup(const std::string& uid) const
{
	_history.call("lookup", uid);
	auto it = _ips.find(uid);
	return it != _ips.end()? it->second : NULL;
}

std::shared_ptr<Peer> MockKnownPeers::lookupAddr(const socket_address& addr) const
{
	_history.call("lookupAddr", addr.toString());
	std::string ip = addr.toString();
	auto it = _ips.find(ip);
	return it != _ips.end()? it->second : NULL;
}

std::shared_ptr<Peer> MockKnownPeers::randomPeer() const
{
	_history.call("randomPeer");
	return _ips.empty()? NULL : _ips.begin()->second;
}

std::string MockKnownPeers::toString() const
{
	return "mock membership";
}


bool MockKnownPeers::save()
{
	_history.call("save");
	return true;
}

