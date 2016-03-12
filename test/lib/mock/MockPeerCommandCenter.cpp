/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockPeerCommandCenter.h"

#include "membership/Peer.h"

void MockPeerCommandCenter::run(const std::shared_ptr<Peer>& peer, const std::string& buffer)
{
	_history.call("run", peer->uid, buffer);
}

void MockPeerCommandCenter::dismiss(const std::shared_ptr<Peer>& peer)
{
	_history.call("dismiss", peer->uid);
}

std::shared_ptr<Turbopump::Command> MockPeerCommandCenter::command(int cid, const char* buff, unsigned size)
{
	_history.call("command", cid, std::string(buff, size));
	return _commands[cid];
}
