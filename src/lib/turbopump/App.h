/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Turbopump.h"
#include "api/Api.h"
#include "api/Options.h"
#include "deskew/ThreadLockedKeyTabulator.h"
#include "main/WanServer.h"
#include "peer_client/MessagePacker.h"
#include "peer_client/MessageSender.h"
#include "peer_client/PartialTransfers.h"
#include "peer_client/WriteSupervisor.h"
#include "peer_server/ConcurrentCommandCenter.h"
#include "peer_server/PeerPacketHandler.h"
#include "programmable/Callbacks.h"

#include "event/SchedulerThread.h"
#include "event/MultiThreadedExecutor.h"

namespace Turbopump {

class App
{
public:
	App(const Options& opts);

	bool run();
	void shutdown();

	const Api& api() const;

protected:
	// shutdown flag
	Event _shutdown;

	// augmented options (this is weird, and needs to be cleaned up)
	Callbacks _opts;

	// core app logic
	Turbopump _turbopump;

	// background thread
	SchedulerThread _scheduler;

	// sync
	ThreadLockedKeyTabulator _threadLockedKeyTabulator;

	// internal comm helpers
	MessagePacker _packer;

	// internal comm client components
	MessageSender _messenger;
	PartialTransfers _partialTransfers;
	WriteSupervisor _writeSupervisor;

	// internal comm server components
	// including threads
	MultiThreadedExecutor _peerExecutor;
	ConcurrentCommandCenter _peerCenter;
	PeerPacketHandler _peerPacketHandler;
	WanServer _peerServer;

};

}//namespace
