/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Turbopump.h"
#include "api/Api.h"
#include "callbacks/ComputedOptions.h"
#include "callbacks/Watches.h"
#include "concurrent/DeadlineScheduler.h"
#include "deskew/SynchronizedKeyTabulator.h"
#include "peer_client/MessagePacker.h"
#include "peer_client/MessageSender.h"
#include "peer_client/PartialTransfers.h"
#include "peer_client/WriteSupervisor.h"
#include "peer_server/ConcurrentCommandCenter.h"
#include "peer_server/PeerPacketHandler.h"
#include "storage/IStore.h"

#include "concurrent/monitor.h"
#include "concurrent/ThreadPoolExecutor.h"
#include "socket/ISocketServer.h"

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
	turbo::monitor _shutdown;

	// data store
	std::unique_ptr<IStore> _store;

	// options
	ComputedOptions _opts;

	// core app logic
	Turbopump _turbopump;

	// background thread
	DeadlineScheduler _scheduler;

	// sync
	SynchronizedKeyTabulator _synchronizedKeyTabulator;

	// internal comm helpers
	MessagePacker _packer;

	// internal comm server
	std::unique_ptr<ISocketServer> _peerServer;

	// internal comm client components
	MessageSender _messenger;
	PartialTransfers _partialTransfers;
	WriteSupervisor _writeSupervisor;

	// internal comm server components
	// including threads
	ThreadPoolExecutor _peerExecutor;
	ConcurrentCommandCenter _peerCenter;
	PeerPacketHandler _peerPacketHandler;
};
}//namespace
