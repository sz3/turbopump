/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WanServer.h"

#include "socket/UdpServer.h"
#include "udt_socket/UdtServer.h"

WanServer::WanServer(const TurboApi::Options& opts, short port, std::function<void(const IIpSocket&, const std::string&)> onPacket)
{
	if (opts.udt)
		_server.reset( new UdtServer(port, onPacket) );
	else
		_server.reset( new UdpServer(port, onPacket) );
}

bool WanServer::start()
{
	return _server->start();
}

void WanServer::stop()
{
	return _server->stop();
}

std::string WanServer::lastError() const
{
	return _server->lastError();
}

std::shared_ptr<IIpSocket> WanServer::sock(const IpAddress& addr)
{
	return _server->sock(addr);
}
