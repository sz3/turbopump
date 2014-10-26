/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "peer_server/IPeerCommandCenter.h"
#include "util/CallHistory.h"
#include <map>

class MockPeerCommandCenter : public IPeerCommandCenter
{
public:
	void run(const std::shared_ptr<Peer>& peer, const std::string& buffer);
	void markFinished(const std::string& id);

	std::shared_ptr<Turbopump::Command> command(int cid, const char* buff, unsigned size);

public:
	CallHistory _history;
	std::map<int,std::shared_ptr<Turbopump::Command>> _commands;
};
