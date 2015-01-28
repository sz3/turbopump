/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "api/Options.h"
#include "socket/ISocketServer.h"
#include <memory>
class IMembership;

class WanServer : public ISocketServer
{
public:
	WanServer(const Turbopump::Options& opts, const socket_address& addr, std::function<void(ISocketWriter&, const char*, unsigned)> onPacket, std::function<bool(ISocketWriter&)> onWriteReady, const IMembership& membership);

	bool start();
	bool stop();

	std::shared_ptr<ISocketWriter> getWriter(const socket_address& endpoint);
	void waitForWriter(const ISocketWriter& writer);

	std::string lastError() const;

protected:
	std::unique_ptr<ISocketServer> _server;
};
