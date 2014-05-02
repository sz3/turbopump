/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockPacketServer.h"

#include "socket/IpAddress.h"

bool MockPacketServer::start()
{
	_history.call("start");
	return true;
}

void MockPacketServer::stop()
{
	_history.call("stop");
}

std::string MockPacketServer::lastError() const
{
	return _error;
}

std::shared_ptr<IIpSocket> MockPacketServer::sock(const IpAddress& addr)
{
	_history.call("sock", addr.toString());
	return _sock;
}
