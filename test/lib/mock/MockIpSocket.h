#pragma once

#include "socket/IIpSocket.h"
#include "socket/IpAddress.h"
#include "util/CallHistory.h"

class MockIpSocket : public IIpSocket
{
public:
	IpAddress getTarget() const;

	int send(const char* buffer, unsigned size) const;
	int recv(std::string& buffer);

public:
	IpAddress _target;
	std::string _recvBuff;

	mutable CallHistory _history;
};
