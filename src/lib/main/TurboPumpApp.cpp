/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "TurboPumpApp.h"

#include "membership/Peer.h"
#include "user_server/UserPacketHandler.h"

#include "http/HttpByteStream.h"
#include "socket/FileByteStream.h"
#include "socket/socket_address.h"
#include <functional>
#include <iostream>
using namespace std::placeholders;

TurboPumpApp::TurboPumpApp(const Turbopump::Options& opts, const std::string& streamSocket, short port)
	: _logger(socket_address("127.0.0.1", port).toString())
	, _reporter(_ring, _membership, _state)
	, _options(opts)
	, _api(_corrector, _localDataStore, _keyLocator, _messenger, _reporter, _synchronizer, _fakeWriter, _options)
	, _callbacks(_options)
	, _keyTabulator(_keyLocator)
	, _threadLockedKeyTabulator(_keyTabulator, _scheduler)
	, _corrector(_keyTabulator, _localDataStore, _messenger, _writeSupervisor, _logger)
	, _synchronizer(_ring, _membership, _keyTabulator, _messenger, _corrector, _logger)
	, _messenger(_packer, _peers)
	, _writeSupervisor(_peers)
	, _membership("turbo_members.txt", socket_address("127.0.0.1", port).toString())
	, _keyLocator(_ring, _membership)
	, _peers(_wanServer)
	, _localServer(socket_address(streamSocket), std::bind(&TurboPumpApp::onClientConnect, this, _1), 2)
	, _wanPacketHandler(_api, _wanExecutor, _membership, _peers, _logger)
	, _wanServer(_options, socket_address("127.0.0.1", port), std::bind(&WanPacketHandler::onPacket, &_wanPacketHandler, _1, _2, _3))
{
	_callbacks.initialize(_ring, _keyLocator, _membership, _threadLockedKeyTabulator, _messenger, _writeSupervisor);
}

void TurboPumpApp::run()
{
	_state.starting();

	// membership
	if (_membership.load())
		_membership.syncToDataStore(_localDataStore);
	else
	{
		std::cerr << "failed to load membership. Warn." << std::endl;
		_membership.addSelf();
	}

	if (_options.partition_keys)
	{
		ConsistentHashRing& ring(_ring);
		auto fun = [&ring] (const Peer& peer) { ring.insert(peer.uid, peer.uid); };
		_membership.forEachPeer(fun);
	}

	// servers
	if (!_wanExecutor.start())
	{
		std::cerr << "failed to start udp handler threads. Abort." << std::endl;
		::exit(-1);
	}

	if (!_wanServer.start())
	{
		std::cerr << "failed to start wan server. Abort. " << _wanServer.lastError() << std::endl;
		::exit(-1);
	}

	if (!_localServer.start())
	{
		std::cerr << "failed to start local socket server. Abort." << std::endl;
		::exit(-1);
	}

	_scheduler.schedulePeriodic(std::bind(&Synchronizer::pingRandomPeer, &_synchronizer), 2000);
	_scheduler.schedulePeriodic(std::bind(&Synchronizer::offloadUnwantedKeys, &_synchronizer), 5000);

	_state.running();
	_shutdown.wait();
	_state.stopping();

	_scheduler.shutdown();
	_localServer.stop();
	_wanServer.stop();
	_wanExecutor.stop();
}

void TurboPumpApp::shutdown()
{
	_shutdown.shutdown();
}

// TODO: split into server class.
void TurboPumpApp::onClientConnect(int fd)
{
	FileByteStream fileStream(fd);
	HttpByteStream httpStream(fileStream);
	Turbopump::Api api(_corrector, _localDataStore, _keyLocator, _messenger, _reporter, _synchronizer, httpStream, _options);
	UserPacketHandler handler(httpStream, api);
	handler.run();
}
