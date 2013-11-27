#include "TurboPumpApp.h"

#include "user_control/Switchboard.h"

#include "event/Event.h"
#include "socket/ConnectionByteStream.h"
#include "socket/IpAddress.h"
#include <functional>
#include <iostream>
#include <signal.h>
using namespace std::placeholders;

namespace {
	Event _shutdown;

	void onShutdown(int sig)
	{
		_shutdown.signal();
	}
}

TurboPumpApp::TurboPumpApp(const TurboApi& instruct, const std::string& streamSocket, short port)
	: _callbacks(instruct)
	, _corrector(_merkleIndex, _localDataStore, _writeActionSender)
	, _synchronizer(_membership, _merkleIndex, _messenger, _corrector)
	, _messenger(_peers)
	, _writeActionSender(_peers)
	, _membership("turbo_members.txt", IpAddress("127.0.0.1", port).toString())
	, _peers(_udpServer)
	, _localServer(streamSocket, std::bind(&TurboPumpApp::onClientConnect, this, _1))
	, _udpPacketHandler(_membership, _peers, _localDataStore, _synchronizer, _callbacks)
	, _udpServer(port, std::bind(&WanPacketHandler::onPacket, &_udpPacketHandler, _1, _2))
{
	_callbacks.initialize(_membership, _peers, _merkleIndex);
}

void TurboPumpApp::run()
{
	::signal(SIGINT, &onShutdown);

	if (!_membership.load())
		std::cerr << "failed to load membership. Warn." << std::endl;

	if (!_udpServer.start())
	{
		std::cerr << "failed to start udp server. Abort. " << _udpServer.lastError() << std::endl;
		::exit(-1);
	}

	if (!_localServer.start())
	{
		std::cerr << "failed to start local socket server. Abort." << std::endl;
		::exit(-1);
	}

	_scheduler.schedulePeriodic(std::bind(&Synchronizer::pingRandomPeer, &_synchronizer), 4000);

	_shutdown.wait();

	_scheduler.shutdown();
	_localServer.stop();
	_udpServer.stop();
}

void TurboPumpApp::shutdown()
{
	_shutdown.signal();
}

// TODO: split into server class.
void TurboPumpApp::onClientConnect(int fd)
{
	ConnectionByteStream stream(fd);
	Switchboard switcher(stream, _localDataStore, _localDataStore, _membership, _callbacks);
	switcher.run();
}
