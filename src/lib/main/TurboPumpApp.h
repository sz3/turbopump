/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "api/Options.h"
#include "turbopump/App.h"

#include "socket/LocalStreamSocketServer.h"
#include <string>

class TurboPumpApp : public Turbopump::App
{
public:
	TurboPumpApp(const Turbopump::Options& opts, const std::string& streamSocket);

	void run();

	void onClientConnect(int fd);

protected:
	LocalStreamSocketServer _localServer;
};
