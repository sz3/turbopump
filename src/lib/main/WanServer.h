/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "programmable/TurboApi.h"
#include "socket/IPacketServer.h"
#include <memory>

class WanServer : public IPacketServer
{
public:
	WanServer(const TurboApi::Options& opts, short port, std::function<void(const IIpSocket&, const std::string&)> onPacket);

	bool start();
	void stop();
	std::string lastError() const;

	std::shared_ptr<IIpSocket> sock(const IpAddress& addr);

protected:
	std::unique_ptr<IPacketServer> _server;
};
