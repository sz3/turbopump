/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ProcessState.h"
#include "StatusReporter.h"
#include "WanServer.h"
#include "api/Api.h"
#include "api/Options.h"
#include "deskew/KeyTabulator.h"
#include "deskew/ThreadLockedKeyTabulator.h"
#include "deskew/SkewCorrector.h"
#include "deskew/Synchronizer.h"
#include "hashing/ConsistentHashRing.h"
#include "hashing/LocateKeys.h"
#include "logging/StderrLogger.h"
#include "membership/Membership.h"
#include "peer_client/MessagePacker.h"
#include "peer_client/MessageSender.h"
#include "peer_client/WriteSupervisor.h"
#include "peer_server/ConcurrentCommandCenter.h"
#include "peer_server/PeerPacketHandler.h"
#include "programmable/Callbacks.h"
#include "storage/FileStore.h"

#include "event/SchedulerThread.h"
#include "event/MultiThreadedExecutor.h"
#include "socket/LocalStreamSocketServer.h"
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
	Callbacks _callbacks;
	Turbopump::Api _api;

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
	FileStore _fileStore;

	// servers!
	LocalStreamSocketServer _localServer;
	MultiThreadedExecutor _peerExecutor;
	//SimpleExecutor _peerExecutor;
	ConcurrentCommandCenter _peerCenter;
	PeerPacketHandler _peerPacketHandler;
	WanServer _peerServer;
};
