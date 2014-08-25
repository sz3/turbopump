/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WanServer.h"

#include "socket/UdpServer.h"
#include "udt_socket/UdtServer.h"

WanServer::WanServer(const TurboApi::Options& opts, const socket_address& addr, std::function<void(ISocketWriter&, const char*, unsigned)> onPacket)
{
	if (opts.udt)
		_server.reset( new UdtServer(addr, onPacket) );
	else
		_server.reset( new UdpServer(addr, onPacket) );
}

bool WanServer::start()
{
	return _server->start();
}

bool WanServer::stop()
{
	return _server->stop();
}

std::shared_ptr<ISocketWriter> WanServer::getWriter(const socket_address& endpoint)
{
	return _server->getWriter(endpoint);
}

std::string WanServer::lastError() const
{
	return _server->lastError();
}
