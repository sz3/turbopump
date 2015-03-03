/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WanServer.h"

#include "peer_server/MultiplexedSocketPool.h"

#include "socket/UdpServer.h"
#include "udt_socket/UdtServer.h"

WanServer::WanServer(const Turbopump::Options& opts, const socket_address& addr, std::function<void(ISocketWriter&, const char*, unsigned)> onPacket, std::function<bool(int)> onWriteReady)
{
	if (opts.udt)
		_server.reset(new UdtServer(addr, onPacket, onWriteReady, new MultiplexedSocketPool<udt_socket>()));
	else
		_server.reset(new UdpServer(addr, onPacket, new MultiplexedSocketPool<udp_socket>()));
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

void WanServer::waitForWriter(int id)
{
	_server->waitForWriter(id);
}

std::string WanServer::lastError() const
{
	return _server->lastError();
}