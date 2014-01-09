/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ProcessState.h"
#include "actions_req/MessageSender.h"
#include "actions_req/WriteActionSender.h"
#include "cohesion/MerkleIndex.h"
#include "cohesion/SkewCorrector.h"
#include "cohesion/Synchronizer.h"
#include "consistent_hashing/HashRing.h"
#include "data_store/LocalDataStore.h"
#include "logging/StderrLogger.h"
#include "membership/Membership.h"
#include "programmable/Callbacks.h"
#include "wan_server/PeerTracker.h"
#include "wan_server/WanPacketHandler.h"

#include "event/SchedulerThread.h"
#include "event/MultiThreadedExecutor.h"
#include "socket/LocalStreamSocketServer.h"
#include "socket/UdpServer.h"
#include <string>

class TurboPumpApp
{
public:
	TurboPumpApp(const TurboApi& instruct, const std::string& streamSocket, short port);

	void run();
	void shutdown();

	void onClientConnect(int fd);

protected:
	Event _shutdown;
	ProcessState _state;
	StderrLogger _logger;

	// plugins
	Callbacks _callbacks;

	// healing
	MerkleIndex _merkleIndex;
	SkewCorrector _corrector;
	Synchronizer _synchronizer;

	// thread scheduling and helpers
	MessageSender _messenger;
	WriteActionSender _writeActionSender;
	SchedulerThread _scheduler;

	// core. hashring, membership, connection tracking, data store...
	HashRing _ring;
	Membership _membership;
	LocalDataStore _localDataStore;
	PeerTracker _peers;

	// servers!
	LocalStreamSocketServer _localServer;
	MultiThreadedExecutor _wanExecutor;
	//SimpleExecutor _wanExecutor;
	WanPacketHandler _wanPacketHandler;
	UdpServer _wanServer;
};
