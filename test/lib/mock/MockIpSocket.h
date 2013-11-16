#pragma once

#include "socket/IIpSocket.h"
#include "socket/IpAddress.h"
#include "util/CallHistory.h"

class MockIpSocket : public IIpSocket
{
public:
	bool setTarget(const IpAddress& address);
	IpAddress getTarget() const;

	int send(const char* buffer, unsigned size) const;
	int recv(std::string& buffer);

public:
	IpAddress _target;
	std::string _recvBuff;

	mutable CallHistory _history;
};
