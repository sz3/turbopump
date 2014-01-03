#include "MockMembership.h"

#include "membership/Peer.h"

bool MockMembership::addIp(const std::string& ip, const std::string& uid)
{
	_history.call("addIp", ip, uid);
	_ips[ip].reset(new Peer(uid));
	return true;
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

std::string MockMembership::toString() const
{
	return "mock membership";
}
