/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "api/Options.h"
#include "turbopump/App.h"
#include "user_server/UserPacketHandler.h"

#include "http/HttpByteStream.h"
#include "socket/FileByteStream.h"
#include "socket/StreamSocketAcceptorServer.h"
#include "socket/local_stream_socket.h"
#include "socket/tcp_socket.h"
#include <functional>
#include <iostream>
#include <memory>

class TurboApp : public Turbopump::App
{
private:
	inline static ISocketServer* localServer(const socket_address& addr, const std::function<void(int)>& onConnect, unsigned threads)
	{
		if (addr.port() == 0)
			return new StreamSocketAcceptorServer<local_stream_socket>(addr, onConnect, threads);
		else
			return new StreamSocketAcceptorServer<tcp_socket>(addr, onConnect, threads);
	}

public:
	TurboApp(const Turbopump::Options& opts, const socket_address& controlAddr)
		: Turbopump::App(opts)
		, _localServer(localServer(controlAddr, [this](int fd){this->onClientConnect(fd);}, 2))
	{}

	void run()
	{
		if (!_localServer->start())
		{
			std::cerr << "failed to start local server. Abort. " << _localServer->lastError() << std::endl;
			::exit(-1);
		}

		Turbopump::App::run();

		_localServer->stop();
	}

	void onClientConnect(int fd)
	{
		FileByteStream fileStream(fd);
		HttpByteStream httpStream(fileStream);
		UserPacketHandler handler(httpStream, api());
		handler.run();
	}

protected:
	std::unique_ptr<ISocketServer> _localServer;
};
