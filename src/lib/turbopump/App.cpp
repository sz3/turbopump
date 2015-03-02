/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "App.h"

#include "socket/socket_address.h"
#include <functional>
using namespace std::placeholders;

namespace Turbopump {

App::App(const Options& opts)
	: _opts(opts)
	, _turbopump(_opts, _messenger, _writeSupervisor)
	, _messenger(_packer, _peerServer)
	, _partialTransfers(_peerServer)
	, _writeSupervisor(_packer, _partialTransfers, _peerServer, _turbopump.store)
	, _peerCenter(_turbopump.api, _peerExecutor)
	, _peerPacketHandler(_turbopump.membership, _peerCenter, _turbopump.logger)
	, _peerServer(_opts, socket_address("127.0.0.1", opts.internal_port), std::bind(&PeerPacketHandler::onPacket, &_peerPacketHandler, _1, _2, _3), std::bind(&PartialTransfers::run, &_partialTransfers, _1))
	, _threadLockedKeyTabulator(_turbopump.keyTabulator, _scheduler)
{
	_opts.initialize(_turbopump.ring, _turbopump.keyLocator, _turbopump.membership, _threadLockedKeyTabulator, _messenger, _writeSupervisor);
}

bool App::run()
{
	_turbopump.state.starting();
	_turbopump.initialize(_opts);

	// start servers
	if (!_peerExecutor.start())
	{
		_turbopump.logger.logError("failed to start wan handler threads. Abort.");
		return false;
	}

	if (!_peerServer.start())
	{
		_turbopump.logger.logError("failed to start wan server. Abort. " + _peerServer.lastError());
		_peerExecutor.stop();
		return false;
	}

	_scheduler.schedulePeriodic(std::bind(&Synchronizer::pingRandomPeer, &_turbopump.synchronizer), 2000);
	_scheduler.schedulePeriodic(std::bind(&Synchronizer::offloadUnwantedKeys, &_turbopump.synchronizer), 5000);

	_turbopump.state.running();
	_shutdown.wait();
	_turbopump.state.stopping();

	_scheduler.shutdown();
	_peerServer.stop();
	_peerExecutor.stop();

	return true;
}

void App::shutdown()
{
	_shutdown.shutdown();
}

const Api& App::api() const
{
	return _turbopump.api;
}

}//namespace
