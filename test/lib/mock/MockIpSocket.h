/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "socket/IIpSocket.h"
#include "socket/IpAddress.h"
#include "util/CallHistory.h"

class MockIpSocket : public IIpSocket
{
public:
	MockIpSocket();

	IpAddress getTarget() const;
	std::string destination() const;

	int try_send(const char* buffer, unsigned size) const;
	int send(const char* buffer, unsigned size) const;
	int recv(std::string& buffer);

public:
	IpAddress _target;
	std::string _recvBuff;
	bool _trySendError;

	mutable CallHistory _history;
};
