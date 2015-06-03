/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "membership/IKnowPeers.h"
#include "membership/Peer.h"
#include "util/CallHistory.h"

#include <map>
#include <memory>
#include <string>

class MockKnownPeers : public IKnowPeers
{
public:
	MockKnownPeers();

	bool update(const std::string& uid, const std::vector<std::string>& ips={});
	bool remove(const std::string& uid);

	std::shared_ptr<Peer> self() const;
	std::shared_ptr<Peer> lookup(const std::string& uid) const;
	std::shared_ptr<Peer> lookupAddr(const socket_address& addr) const;
	std::shared_ptr<Peer> randomPeer() const;

	std::string toString() const;
	
	bool save();

public:
	mutable CallHistory _history;
	std::shared_ptr<Peer> _self;
	std::map< std::string, std::shared_ptr<Peer> > _ips;
};

