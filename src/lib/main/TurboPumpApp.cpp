/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "TurboPumpApp.h"

#include "membership/Peer.h"
#include "user_control/Switchboard.h"

#include "socket/FileByteStream.h"
#include "socket/IpAddress.h"
#include <functional>
#include <iostream>
using namespace std::placeholders;

TurboPumpApp::TurboPumpApp(const TurboApi& instruct, const std::string& streamSocket, short port)
	: _logger(IpAddress("127.0.0.1", port).toString())
	, _callbacks(instruct)
	, _merkleIndex(_ring, _membership)
	, _threadLockedMerkleIndex(_merkleIndex, _scheduler)
	, _corrector(_merkleIndex, _localDataStore, _writeActionSender)
	, _synchronizer(_ring, _membership, _merkleIndex, _messenger, _corrector)
	, _messenger(_peers)
	, _writeActionSender(_peers)
	, _membership("turbo_members.txt", IpAddress("127.0.0.1", port).toString())
	, _peers(_wanServer)
	, _localServer(streamSocket, std::bind(&TurboPumpApp::onClientConnect, this, _1), 2)
	, _wanPacketHandler(_wanExecutor, _ring, _membership, _peers, _localDataStore, _synchronizer, _logger, _callbacks)
	, _wanServer(instruct.options, port, std::bind(&WanPacketHandler::onPacket, &_wanPacketHandler, _1, _2))
{
	_callbacks.initialize(_ring, _membership, _threadLockedMerkleIndex, _messenger, _peers);
}

void TurboPumpApp::run()
{
	_state.starting();
	if (!_membership.load())
		std::cerr << "failed to load membership. Warn." << std::endl;
	else
	{
		_membership.syncToDataStore(_localDataStore);
		if (_callbacks.options.partition_keys)
		{
			HashRing& ring(_ring);
			auto fun = [&ring] (const Peer& peer) { ring.addWorker(peer.uid); };
			_membership.forEachPeer(fun);
		}
	}

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
	FileByteStream stream(fd);
	Switchboard switcher(stream, _localDataStore, _ring, _membership, _threadLockedMerkleIndex, _state, _callbacks);
	switcher.run();
}
