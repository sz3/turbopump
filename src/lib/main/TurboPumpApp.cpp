/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "TurboPumpApp.h"

#include "user_server/UserPacketHandler.h"

#include "http/HttpByteStream.h"
#include "socket/FileByteStream.h"
#include "socket/StreamSocketAcceptorServer.h"
#include "socket/local_stream_socket.h"
#include "socket/socket_address.h"
#include "socket/tcp_socket.h"
#include <functional>
#include <iostream>

using namespace std::placeholders;

namespace {
	// could be in a separate compilation unit. Point is: it's a factory method.
	ISocketServer* localServer(const socket_address& addr, const std::function<void(int)>& onConnect, unsigned threads)
	{
		if (addr.port() == 0)
			return new StreamSocketAcceptorServer<local_stream_socket>(addr, onConnect, threads);
		else
			return new StreamSocketAcceptorServer<tcp_socket>(addr, onConnect, threads);
	}
}

TurboPumpApp::TurboPumpApp(const Turbopump::Options& opts, const socket_address& controlAddr)
	: Turbopump::App(opts)
	, _localServer(localServer(controlAddr, std::bind(&TurboPumpApp::onClientConnect, this, _1), 2))
{
}

void TurboPumpApp::run()
{
	if (!_localServer->start())
	{
		std::cerr << "failed to start local server. Abort. " << _localServer->lastError() << std::endl;
		::exit(-1);
	}

	Turbopump::App::run();

	_localServer->stop();
}

// TODO: split into server class.
void TurboPumpApp::onClientConnect(int fd)
{
	FileByteStream fileStream(fd);
	HttpByteStream httpStream(fileStream);
	UserPacketHandler handler(httpStream, api());
	handler.run();
}
