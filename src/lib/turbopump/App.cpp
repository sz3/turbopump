/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "App.h"

#include "Interface.h"
#include "callbacks/ComputedOptions.h"
#include "peer_server/MultiplexedSocketPool.h"
#include "storage/FileStore.h"
#include "storage/SimpleFileStore.h"

#include "serialize/str.h"
#include "socket/socket_address.h"
#include "socket/UdpServer.h"
#include "udt_socket/UdtServer.h"
#include <functional>
#include <vector>
using namespace std::placeholders;

namespace {
	// could be in a separate compilation unit. Point is: these are factory methods.

	ISocketServer* peerServer(const Turbopump::Options& opts, const socket_address& addr)
	{
		if (opts.udt)
			return new UdtServer(addr, new MultiplexedSocketPool<udt_socket>());
		else
			return new UdpServer(addr, new MultiplexedSocketPool<udp_socket>());
	}

	IStore* dataStore(const Turbopump::Options& opts)
	{
		std::string type;
		std::string path;

		std::vector<std::string> parts = turbo::str::split(opts.store, ':');
		if ( !parts.empty() )
		{
			type = parts.front();
			if (parts.size() >= 2)
				path = parts[1];
		}

		if (path.empty())
			path = opts.home_dir + "/store";
		else if (path.front() != '/')
			path = opts.home_dir + "/" + path;

		if (type == "file")
			return new SimpleFileStore(path);

		// default
		return new FileStore(path);
	}
}

namespace Turbopump {
App::App(const Options& opts)
	: _store(dataStore(opts))
	, _opts(opts, Interface{_turbopump.api, _turbopump.logger, *_store, _turbopump.membership,
				   _turbopump.ring, _turbopump.keyLocator, _synchronizedKeyTabulator,
				   _turbopump.corrector, _turbopump.synchronizer, _messenger, _writeSupervisor,
				   _synchronizedWatches})
	, _turbopump(_opts, *_store, _messenger, _writeSupervisor)
	, _peerServer(peerServer(opts, socket_address("127.0.0.1", opts.internal_port)))
	, _messenger(_packer, *_peerServer)
	, _partialTransfers(*_peerServer)
	, _writeSupervisor(_packer, _partialTransfers, *_peerServer, _turbopump.store)
	, _peerCenter(_turbopump.api, _peerExecutor)
	, _peerPacketHandler(_turbopump.membership, _peerCenter, _turbopump.logger)
	, _synchronizedKeyTabulator(_turbopump.keyTabulator, _scheduler)
	, _synchronizedWatches(_turbopump.watches, _scheduler)
{
}

bool App::run()
{
	_turbopump.state.starting();
	_turbopump.preStart(_opts);

	// start servers
	if (!_peerExecutor.start())
	{
		_turbopump.logger.logError("failed to start wan handler threads. Abort.");
		return false;
	}

	if (!_peerServer->start(std::bind(&PeerPacketHandler::onPacket, &_peerPacketHandler, _1, _2, _3), std::bind(&PartialTransfers::run, &_partialTransfers, _1)))
	{
		_turbopump.logger.logError("failed to start wan server. Abort. " + _peerServer->lastError());
		_peerExecutor.stop();
		return false;
	}

	_turbopump.postStart(_opts);

	_scheduler.schedule_repeat(std::bind(&Synchronizer::pingRandomPeer, &_turbopump.synchronizer), _opts.sync_interval_ms);
	_scheduler.schedule_repeat(std::bind(&Synchronizer::offloadUnwantedKeys, &_turbopump.synchronizer), _opts.offload_interval_ms);

	_turbopump.state.running();
	_shutdown.wait();
	_turbopump.state.stopping();

	_scheduler.shutdown();
	_peerServer->stop();
	_peerExecutor.stop();

	return true;
}

void App::shutdown()
{
	// callable by signal handlers.
	// e.g. do not attempt to wait on mutexes.
	_shutdown.notify_all();
}

const Api& App::api() const
{
	return _turbopump.api;
}

}//namespace
