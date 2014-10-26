/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "PeerPacketHandler.h"
#include "mock/MockMembership.h"
#include "mock/MockPeerCommandCenter.h"
#include "mock/MockLogger.h"
#include "socket/MockSocketWriter.h"

TEST_CASE( "PeerPacketHandlerTest/testOnPacket", "[unit]" )
{
	MockMembership membership;
	membership.addIp("1.2.3.4", "fooid");
	membership._history.clear();
	MockPeerCommandCenter center;
	MockLogger logger;
	PeerPacketHandler handler(membership, center, logger);

	MockSocketWriter writer;
	writer._endpoint = socket_address("1.2.3.4", 5);
	assertMsg( handler.onPacket(writer, "foobar", 6), logger._history.calls() );

	assertEquals( "lookupIp(1.2.3.4)", membership._history.calls() );
	assertEquals( "run(fooid,foobar)", center._history.calls() );
	assertEquals( "", logger._history.calls() );
}

TEST_CASE( "PeerPacketHandlerTest/testOnPacket.BadPeer", "[unit]" )
{
	MockMembership membership;
	MockPeerCommandCenter center;
	MockLogger logger;
	PeerPacketHandler handler(membership, center, logger);

	MockSocketWriter writer;
	writer._endpoint = socket_address("8.8.8.8", 12);
	assertFalse( handler.onPacket(writer, "foobar", 6) );

	assertEquals( "lookupIp(8.8.8.8)", membership._history.calls() );
	assertEquals( "logWarn(rejecting packet from unknown host 8.8.8.8:12)", logger._history.calls() );
	assertEquals( "", center._history.calls() );
}

TEST_CASE( "PeerPacketHandlerTest/testOnPacket.Empty", "[unit]" )
{
	MockMembership membership;
	membership.addIp("8.8.8.8", "fooid");
	membership._history.clear();
	MockPeerCommandCenter center;
	MockLogger logger;
	PeerPacketHandler handler(membership, center, logger);

	MockSocketWriter writer;
	writer._endpoint = socket_address("8.8.8.8", 12);
	assertFalse( handler.onPacket(writer, NULL, 0) );

	assertEquals( "lookupIp(8.8.8.8)", membership._history.calls() );
	assertEquals( "", logger._history.calls() );
	assertEquals( "", center._history.calls() );
}

