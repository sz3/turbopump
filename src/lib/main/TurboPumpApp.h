#pragma once

#include "actions_req/MessageSender.h"
#include "actions_req/WriteActionSender.h"
#include "cohesion/MerkleIndex.h"
#include "cohesion/SkewCorrector.h"
#include "cohesion/Synchronizer.h"
#include "data_store/LocalDataStore.h"
#include "membership/Membership.h"
#include "programmable/Callbacks.h"
#include "wan_server/PeerTracker.h"
#include "wan_server/WanPacketHandler.h"

#include "event/SchedulerThread.h"
#include "socket/LocalStreamSocketServer.h"
#include "socket/UdpServer.h"
#include <string>

class TurboPumpApp
{
public:
	TurboPumpApp(const TurboApi& instruct, const std::string& streamSocket, short port);

	void run();
	void shutdown(); // alternate hook for tests

	void onClientConnect(int fd);

protected:
	// core. Membership, connection tracking, data store...
	Membership _membership;
	PeerTracker _peers;
	LocalDataStore _localDataStore;

	// servers!
	LocalStreamSocketServer _localServer;
	WanPacketHandler _udpPacketHandler;
	UdpServer _udpServer;

	// thread scheduling and helpers
	MessageSender _messenger;
	WriteActionSender _writeActionSender;
	SchedulerThread _scheduler;

	// healing
	MerkleIndex _merkleIndex;
	SkewCorrector _corrector;
	Synchronizer _synchronizer;

	// plugins
	Callbacks _callbacks;
};
