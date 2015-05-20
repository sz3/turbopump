/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "App.h"

#include "Interface.h"
#include "callbacks/BuildCallbacks.h"
#include "peer_server/MultiplexedSocketPool.h"

#include "socket/socket_address.h"
#include "socket/UdpServer.h"
#include "udt_socket/UdtServer.h"
#include <functional>
using namespace std::placeholders;

namespace {
	// could be in a separate compilation unit. Point is: it's a factory method.
	// local store will be done the same way!
	// probably. Results of -fdevirtualize will determine whether template nonsense is necessary, hopefully not.

	ISocketServer* peerServer(const Turbopump::Options& opts, const socket_address& addr, std::function<void(ISocketWriter&, const char*, unsigned)> onPacket, std::function<bool(int)> onWriteReady)
	{
		if (opts.udt)
			return new UdtServer(addr, onPacket, onWriteReady, new MultiplexedSocketPool<udt_socket>());
		else
			return new UdpServer(addr, onPacket, new MultiplexedSocketPool<udp_socket>());
	}
}

namespace Turbopump {
App::App(const Options& opts)
	: _opts(opts)
	, _turbopump(_opts, _messenger, _writeSupervisor)
	, _peerServer(peerServer(opts, socket_address("127.0.0.1", opts.internal_port), std::bind(&PeerPacketHandler::onPacket, &_peerPacketHandler, _1, _2, _3), std::bind(&PartialTransfers::run, &_partialTransfers, _1)))
	, _messenger(_packer, *_peerServer)
	, _partialTransfers(*_peerServer)
	, _writeSupervisor(_packer, _partialTransfers, *_peerServer, _turbopump.store)
	, _peerCenter(_turbopump.api, _peerExecutor)
	, _peerPacketHandler(_turbopump.membership, _peerCenter, _turbopump.logger)
	, _threadLockedKeyTabulator(_turbopump.keyTabulator, _scheduler)
{
	Interface iface{_turbopump.api, _turbopump.logger, _turbopump.store, _turbopump.membership, _turbopump.ring,
		_turbopump.keyLocator, _threadLockedKeyTabulator, _turbopump.corrector, _turbopump.synchronizer, _messenger, _writeSupervisor};
	BuildCallbacks(_opts).build(iface);
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

	if (!_peerServer->start())
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
