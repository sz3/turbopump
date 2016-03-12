/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "PeerPacketHandler.h"
#include "mock/MockKnownPeers.h"
#include "mock/MockPeerCommandCenter.h"
#include "mock/MockLogger.h"
#include "socket/MockSocketWriter.h"

TEST_CASE( "PeerPacketHandlerTest/testOnPacket", "[unit]" )
{
	MockKnownPeers membership;
	membership.update("fooid", {"1.2.3.4:5"});
	membership._history.clear();
	MockPeerCommandCenter center;
	MockLogger logger;
	PeerPacketHandler handler(membership, center, logger);

	MockSocketWriter writer;
	writer._endpoint = socket_address("1.2.3.4", 5);
	handler.onPacket(writer, "foobar", 6);

	assertEquals( "lookupAddr(1.2.3.4:5)", membership._history.calls() );
	assertEquals( "run(fooid,foobar)", center._history.calls() );
	assertEquals( "", logger._history.calls() );
}

TEST_CASE( "PeerPacketHandlerTest/testOnPacket.BadPeer", "[unit]" )
{
	MockKnownPeers membership;
	MockPeerCommandCenter center;
	MockLogger logger;
	PeerPacketHandler handler(membership, center, logger);

	MockSocketWriter writer;
	writer._endpoint = socket_address("8.8.8.8", 12);
	handler.onPacket(writer, "foobar", 6);

	assertEquals( "lookupAddr(8.8.8.8:12)", membership._history.calls() );
	assertEquals( "logWarn(rejecting packet from unknown host 8.8.8.8:12)", logger._history.calls() );
	assertEquals( "", center._history.calls() );
}

TEST_CASE( "PeerPacketHandlerTest/testOnPacket.Empty", "[unit]" )
{
	MockKnownPeers membership;
	membership.update("fooid", {"8.8.8.8:12"});
	membership._history.clear();
	MockPeerCommandCenter center;
	MockLogger logger;
	PeerPacketHandler handler(membership, center, logger);

	MockSocketWriter writer;
	writer._endpoint = socket_address("8.8.8.8", 12);
	handler.onPacket(writer, NULL, 0);

	assertEquals( "lookupAddr(8.8.8.8:12)", membership._history.calls() );
	assertEquals( "dismiss(fooid)", center._history.calls() );
	assertEquals( "", logger._history.calls() );
}
