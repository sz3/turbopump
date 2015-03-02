/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "TurboPumpApp.h"

#include "user_server/UserPacketHandler.h"

#include "http/HttpByteStream.h"
#include "socket/FileByteStream.h"
#include "socket/socket_address.h"
#include <functional>
#include <iostream>

using namespace std::placeholders;

TurboPumpApp::TurboPumpApp(const Turbopump::Options& opts, const std::string& streamSocket)
	: Turbopump::App(opts)
	, _localServer(socket_address(streamSocket), std::bind(&TurboPumpApp::onClientConnect, this, _1), 2)
{
}

void TurboPumpApp::run()
{
	if (!_localServer.start())
	{
		std::cerr << "failed to start local socket server. Abort." << std::endl;
		::exit(-1);
	}

	Turbopump::App::run();

	_localServer.stop();
}

// TODO: split into server class.
void TurboPumpApp::onClientConnect(int fd)
{
	FileByteStream fileStream(fd);
	HttpByteStream httpStream(fileStream);
	UserPacketHandler handler(httpStream, api());
	handler.run();
}
