/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ProcessState.h"
#include "StatusReporter.h"
#include "WanServer.h"
#include "actions_req/MessagePacker.h"
#include "actions_req/MessageSender.h"
#include "actions_req/WriteSupervisor.h"
#include "api/Api.h"
#include "api/Options.h"
#include "data_store/RamDataStore.h"
#include "deskew/KeyTabulator.h"
#include "deskew/ThreadLockedKeyTabulator.h"
#include "deskew/SkewCorrector.h"
#include "deskew/Synchronizer.h"
#include "hashing/ConsistentHashRing.h"
#include "hashing/LocateKeys.h"
#include "logging/StderrLogger.h"
#include "membership/Membership.h"
#include "programmable/Callbacks.h"
#include "wan_server/PeerTracker.h"
#include "wan_server/WanPacketHandler.h"

#include "event/SchedulerThread.h"
#include "event/MultiThreadedExecutor.h"
#include "socket/LocalStreamSocketServer.h"
#include "socket/NullByteStream.h"
#include <string>

class TurboPumpApp
{
public:
	TurboPumpApp(const Turbopump::Options& opts, const std::string& streamSocket, short port);

	void run();
	void shutdown();

	void onClientConnect(int fd);

protected:
	Event _shutdown;
	StderrLogger _logger;
	ProcessState _state;
	StatusReporter _reporter;

	// options, api
	Turbopump::Options _options;
	Turbopump::Api _api;
	NullByteStream _fakeWriter;

	// plugins
	Callbacks _callbacks;

	// healing
	KeyTabulator _keyTabulator;
	ThreadLockedKeyTabulator _threadLockedKeyTabulator;
	SkewCorrector _corrector;
	Synchronizer _synchronizer;

	// thread scheduling and helpers
	MessagePacker _packer;
	MessageSender _messenger;
	WriteSupervisor _writeSupervisor;
	SchedulerThread _scheduler;

	// core. hashring, membership, connection tracking, data store...
	ConsistentHashRing _ring;
	Membership _membership;
	LocateKeys _keyLocator;
	RamDataStore _localDataStore;
	PeerTracker _peers;

	// servers!
	LocalStreamSocketServer _localServer;
	MultiThreadedExecutor _wanExecutor;
	//SimpleExecutor _wanExecutor;
	WanPacketHandler _wanPacketHandler;
	WanServer _wanServer;
};
