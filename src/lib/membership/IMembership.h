/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
#include <string>
#include <vector>

class Peer;

class IMembership
{
public:
	virtual ~IMembership() {}

	virtual bool addIp(const std::string& ip, const std::string& uid) = 0;
	virtual std::shared_ptr<Peer> lookup(const std::string& uid) const = 0;
	virtual std::shared_ptr<Peer> lookupIp(const std::string& ip) const = 0;
	virtual std::shared_ptr<Peer> self() const = 0;
	virtual std::shared_ptr<Peer> randomPeer() const = 0;
	virtual std::shared_ptr<Peer> randomPeerFromList(std::vector<std::string> list) const = 0;

	virtual std::string toString() const = 0;
};
