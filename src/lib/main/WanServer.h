/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "programmable/TurboApi.h"
#include "socket/ISocketServer.h"
#include <memory>

class WanServer : public ISocketServer
{
public:
	WanServer(const TurboApi::Options& opts, short port, std::function<void(ISocketWriter&, const char*, unsigned)> onPacket);

	bool start();
	bool stop();

	std::shared_ptr<ISocketWriter> getWriter(const IpAddress& endpoint);

	std::string lastError() const;

protected:
	std::unique_ptr<ISocketServer> _server;
};
