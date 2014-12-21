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
	, _api(_corrector, _keyLocator, _messenger, _reporter, _fileStore, _synchronizer, _callbacks)
	, _callbacks(opts)
	, _keyTabulator(_keyLocator)
	, _threadLockedKeyTabulator(_keyTabulator, _scheduler)
	, _corrector(_keyTabulator, _fileStore, _messenger, _writeSupervisor, _logger)
	, _synchronizer(_ring, _membership, _keyTabulator, _messenger, _corrector, _logger)
	, _messenger(_packer, _peerServer)
	, _writeSupervisor(_packer, _peerServer)
	, _membership("turbo_members.txt", socket_address("127.0.0.1", port).toString())
	, _keyLocator(_ring, _membership)
	, _fileStore(StringUtil::str(port) + "/store")
	, _localServer(socket_address(streamSocket), std::bind(&TurboPumpApp::onClientConnect, this, _1), 2)
	, _peerCenter(_api, _peerExecutor)
	, _peerPacketHandler(_membership, _peerCenter, _logger)
	, _peerServer(_callbacks, socket_address("127.0.0.1", port), std::bind(&PeerPacketHandler::onPacket, &_peerPacketHandler, _1, _2, _3), _membership)
{
	_callbacks.initialize(_ring, _keyLocator, _membership, _threadLockedKeyTabulator, _messenger, _writeSupervisor);
}

void TurboPumpApp::run()
{
	_state.starting();

	// membership
	if (_membership.load())
		_membership.syncToDataStore(_fileStore);
	else
	{
		std::cerr << "failed to load membership. Warn." << std::endl;
		_membership.addSelf();
	}

	if (_callbacks.partition_keys)
	{
		ConsistentHashRing& ring(_ring);
		auto fun = [&ring] (const Peer& peer) { ring.insert(peer.uid, peer.uid); };
		_membership.forEachPeer(fun);
	}

	// servers
	if (!_peerExecutor.start())
	{
		std::cerr << "failed to start wan handler threads. Abort." << std::endl;
		::exit(-1);
	}

	if (!_peerServer.start())
	{
		std::cerr << "failed to start wan server. Abort. " << _peerServer.lastError() << std::endl;
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
	_peerServer.stop();
	_peerExecutor.stop();
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
	UserPacketHandler handler(httpStream, _api);
	handler.run();
}
