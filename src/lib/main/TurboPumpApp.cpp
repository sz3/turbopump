#include "TurboPumpApp.h"

#include "user_control/Switchboard.h"

#include "socket/FileByteStream.h"
#include "socket/IpAddress.h"
#include <functional>
#include <iostream>
using namespace std::placeholders;

TurboPumpApp::TurboPumpApp(const TurboApi& instruct, const std::string& streamSocket, short port)
	: _callbacks(instruct)
	, _corrector(_merkleIndex, _localDataStore, _writeActionSender)
	, _synchronizer(_membership, _merkleIndex, _messenger, _corrector)
	, _messenger(_peers)
	, _writeActionSender(_peers)
	, _membership("turbo_members.txt", IpAddress("127.0.0.1", port).toString())
	, _peers(_wanServer)
	, _localServer(streamSocket, std::bind(&TurboPumpApp::onClientConnect, this, _1), 2)
	, _wanPacketHandler(_wanExecutor, _membership, _peers, _localDataStore, _synchronizer, _callbacks)
	, _wanServer(port, std::bind(&WanPacketHandler::onPacket, &_wanPacketHandler, _1, _2))
{
	_callbacks.initialize(_membership, _peers, _merkleIndex);
}

void TurboPumpApp::run()
{
	if (!_membership.load())
		std::cerr << "failed to load membership. Warn." << std::endl;

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

	_scheduler.schedulePeriodic(std::bind(&Synchronizer::pingRandomPeer, &_synchronizer), 3000);

	_shutdown.wait();

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
	Switchboard switcher(stream, _localDataStore, _membership, _callbacks);
	switcher.run();
}
