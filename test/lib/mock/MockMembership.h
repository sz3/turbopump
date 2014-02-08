/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "membership/IMembership.h"
#include "util/CallHistory.h"
#include <map>

class MockMembership : public IMembership
{
public:
	MockMembership();

	bool save();

	bool add(const std::string& uid);
	bool addIp(const std::string& ip, const std::string& uid);
	bool remove(const std::string& uid);

	std::shared_ptr<Peer> lookup(const std::string& uid) const;
	std::shared_ptr<Peer> lookupIp(const std::string& ip) const;
	std::shared_ptr<Peer> self() const;
	bool containsSelf(const std::vector<std::string>& list) const;

	std::shared_ptr<Peer> randomPeer() const;
	std::shared_ptr<Peer> randomPeerFromList(std::vector<std::string> list) const;

	std::string toString() const;

public:
	mutable CallHistory _history;
	std::shared_ptr<Peer> _self;
	bool _containsSelf;
	std::map< std::string,std::shared_ptr<Peer> > _ips;
};

