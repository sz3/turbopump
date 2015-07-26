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
	inline ISocketServer* localServer(const socket_address& addr)
	{
		if (addr.port() == 0)
			return new StreamSocketAcceptorServer<local_stream_socket>(addr, [this](local_stream_socket sock){this->onClientConnect(sock);});
		else
			return new StreamSocketAcceptorServer<tcp_socket>(addr, [this](tcp_socket sock){this->onClientConnect(sock);});
	}

public:
	TurboApp(const Turbopump::Options& opts, const socket_address& controlAddr)
		: Turbopump::App(opts)
		, _localServer(localServer(controlAddr))
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

	template <typename Socket>
	void onClientConnect(Socket& sock)
	{
		auto fun = [this, sock] () mutable
		{
			FileByteStream fileStream(sock.handle());
			HttpByteStream httpStream(fileStream);
			UserPacketHandler handler(httpStream, this->api());
			handler.run();
			sock.close();
		};
		std::thread(fun).detach();
	}

protected:
	std::unique_ptr<ISocketServer> _localServer;
};
