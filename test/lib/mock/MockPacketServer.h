/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "socket/IPacketServer.h"
#include "util/CallHistory.h"

class MockPacketServer : public IPacketServer
{
public:
	bool start();
	void stop();
	std::string lastError() const;

	std::shared_ptr<IIpSocket> sock(const IpAddress& addr);

public:
	CallHistory _history;
	std::shared_ptr<IIpSocket> _sock;
	std::string _error;
};
