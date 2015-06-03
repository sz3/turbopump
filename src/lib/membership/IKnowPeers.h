/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
#include <string>
#include <vector>

class Peer;
class socket_address;

class IKnowPeers
{
public:
	virtual ~IKnowPeers() {}

	virtual bool update(const std::string& uid, const std::vector<std::string>& ips) = 0;
	virtual bool remove(const std::string& uid) = 0;

	virtual std::shared_ptr<Peer> self() const = 0;
	virtual std::shared_ptr<Peer> lookup(const std::string& uid) const = 0;
	virtual std::shared_ptr<Peer> lookupAddr(const socket_address& addr) const = 0;
	virtual std::shared_ptr<Peer> randomPeer() const = 0;

	virtual std::string toString() const = 0;
	
	virtual bool save() = 0;
};

