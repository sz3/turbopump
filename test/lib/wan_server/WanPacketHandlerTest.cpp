/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "WanPacketHandler.h"

#include "membership/Peer.h"
#include "mock/MockDataStore.h"
#include "mock/MockConsistentHashRing.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockLogger.h"
#include "mock/MockMembership.h"
#include "mock/MockMessageSender.h"
#include "mock/MockPeerTracker.h"
#include "mock/MockSkewCorrector.h"
#include "mock/MockSynchronize.h"
#include "programmable/TurboApi.h"
#include "wan_server/PeerConnection.h"

#include "event/SimpleExecutor.h"
#include "socket/MockSocketWriter.h"
#include "socket/socket_address.h"
using std::string;

namespace {
	string formatPacket(unsigned char virtid, const string& packet)
	{
		string header{0, (char)(packet.size()+1), (char)virtid};
		return header + packet;
	}
}

TEST_CASE( "WanPacketHandlerTest/testProcessPendingBuffers", "default" )
{
	SimpleExecutor executor;
	MockDataStore dataStore; // ahead of PeerTracker to avoid double free when test fails. :)
	MockConsistentHashRing ring;
	MockLocateKeys locator;
	MockMembership membership;
	MockMessageSender messenger;
	MockLogger logger;
	MockPeerTracker peers;
	MockSkewCorrector corrector;
	MockSynchronize sync;
	TurboApi callbacks;
	WanPacketHandler handler(executor, corrector, dataStore, ring, locator, membership, messenger, peers, sync, logger, callbacks);

	PeerConnection conn;
	conn.pushRecv(formatPacket(33, "garbage bad action lulz||"));
	conn.pushRecv(formatPacket(33, "key-req|first=1 last=10|"));
	conn.pushRecv(formatPacket(33, "key-req|first=2 last=20|"));
	conn.pushRecv(formatPacket(33, "key-req|first=3 last=30|"));

	handler.processPendingBuffers(Peer("someguid"), conn);
	assertTrue( conn.empty() );
	assertEquals( "pushKeyRange(someguid,,3,1,10,)|pushKeyRange(someguid,,3,2,20,)|pushKeyRange(someguid,,3,3,30,)", corrector._history.calls() );
	assertTrue( !conn.action(33) );
}

TEST_CASE( "WanPacketHandlerTest/testProcessPendingBuffers_ConcurrentFileWrite", "default" )
{
	SimpleExecutor executor;
	MockDataStore dataStore; // ahead of PeerTracker to avoid double free when test fails. :)
	MockConsistentHashRing ring;
	MockLocateKeys locator;
	MockMembership membership;
	MockMessageSender messenger;
	MockLogger logger;
	MockPeerTracker peers;
	MockSkewCorrector corrector;
	MockSynchronize sync;
	TurboApi callbacks;
	WanPacketHandler handler(executor, corrector, dataStore, ring, locator, membership, messenger, peers, sync, logger, callbacks);

	PeerConnection conn;
	conn.pushRecv(formatPacket(33, "write|name=foo|i am a file"));
	conn.pushRecv(formatPacket(35, "write|name=bar|i am a different file"));
	conn.pushRecv(formatPacket(35, " with different bytes"));
	conn.pushRecv(formatPacket(33, " with more bytes!"));
	conn.pushRecv(formatPacket(35, ""));
	conn.pushRecv(formatPacket(33, ""));

	handler.processPendingBuffers(Peer("someguid"), conn);
	assertTrue( conn.empty() );
	assertEquals( "i am a file with more bytes!", dataStore._store["foo"] );
	assertEquals( "i am a different file with different bytes", dataStore._store["bar"] );
}

TEST_CASE( "WanPacketHandlerTest/testProcessPendingBuffers_ReuseOldVirtid", "default" )
{
	SimpleExecutor executor;
	MockDataStore dataStore; // ahead of PeerTracker to avoid double free when test fails. :)
	MockConsistentHashRing ring;
	MockLocateKeys locator;
	MockMembership membership;
	MockMessageSender messenger;
	MockLogger logger;
	MockPeerTracker peers;
	MockSkewCorrector corrector;
	MockSynchronize sync;
	TurboApi callbacks;
	WanPacketHandler handler(executor, corrector, dataStore, ring, locator, membership, messenger, peers, sync, logger, callbacks);

	PeerConnection conn;
	conn.pushRecv(formatPacket(33, "write|name=foo|i am a file"));
	conn.pushRecv(formatPacket(33, ""));
	conn.pushRecv(formatPacket(33, "write|name=bar|i am a different file"));
	conn.pushRecv(formatPacket(33, ""));

	handler.processPendingBuffers(Peer("someguid"), conn);
	assertTrue( conn.empty() );
	assertEquals( "i am a file", dataStore._store["foo"] );
	assertEquals( "i am a different file", dataStore._store["bar"] );
}

TEST_CASE( "WanPacketHandlerTest/testOnPacket", "default" )
{
	SimpleExecutor executor;
	MockDataStore dataStore; // ahead of PeerTracker to avoid double free when test fails. :)
	MockConsistentHashRing ring;
	MockLocateKeys locator;
	MockMembership membership;
	MockMessageSender messenger;
	MockLogger logger;
	MockPeerTracker peers;
	MockSkewCorrector corrector;
	MockSynchronize sync;
	TurboApi callbacks;
	WanPacketHandler handler(executor, corrector, dataStore, ring, locator, membership, messenger, peers, sync, logger, callbacks);

	MockSocketWriter sock;
	sock._endpoint = socket_address("1.2.3.4", 10);
	peers._conn.reset(new PeerConnection);

	string packet = formatPacket(32, "foo");
	assertFalse( handler.onPacket(sock, packet.data(), packet.size()) );
	assertEquals( "", peers._history.calls() );

	membership._ips["1.2.3.4"].reset(new Peer("someguid"));
	assertTrue( handler.onPacket(sock, packet.data(), packet.size()) );
	assertEquals( "track(someguid)", peers._history.calls() );

	peers._history.clear();

	// finally, send a properly formatted packet
	packet = formatPacket(33, "key-req|first=1 last=10|");
	assertTrue( handler.onPacket(sock, packet.data(), packet.size()) );
	assertEquals( "track(someguid)", peers._history.calls() );
	assertEquals( "pushKeyRange(someguid,,3,1,10,)", corrector._history.calls() );
}

TEST_CASE( "WanPacketHandlerTest/testOnPacketMultiplexing", "default" )
{
	SimpleExecutor executor;
	MockDataStore dataStore;
	MockConsistentHashRing ring;
	MockLocateKeys locator;
	MockMembership membership;
	MockMessageSender messenger;
	MockLogger logger;
	MockPeerTracker peers;
	MockSkewCorrector corrector;
	MockSynchronize sync;
	TurboApi callbacks;
	WanPacketHandler handler(executor, corrector, dataStore, ring, locator, membership, messenger, peers, sync, logger, callbacks);

	MockSocketWriter sock;
	sock._endpoint = socket_address("1.2.3.4", 10);
	peers._conn.reset(new PeerConnection);
	membership._ips["1.2.3.4"].reset(new Peer("someguid"));

	string packet = formatPacket(35, "write|name=foo|i am a file") + formatPacket(37, "write|name=bar|another file") + formatPacket(37, "") + formatPacket(35, "");
	assertTrue( handler.onPacket(sock, packet.data(), packet.size()) );
	assertEquals( "track(someguid)", peers._history.calls() );

	// work through both writes
	assertEquals( "i am a file", dataStore._store["foo"] );
	assertEquals( "another file", dataStore._store["bar"] );
	assertEquals( "track(someguid)", peers._history.calls() );
}


/*TEST_CASE( "WanPacketHandlerTest/testOnPacket_RecoverOnRetransmit", "default" )
{
	SimpleExecutor executor;
	MockDataStore dataStore; // ahead of PeerTracker to avoid double free when test fails. :)
	MockConsistentHashRing ring;
	MockLocateKeys locator;
	MockMembership membership;
	MockMessageSender messenger;
	MockLogger logger;
	MockPeerTracker peers;
	MockSkewCorrector corrector;
	MockSynchronize sync;
	TurboApi callbacks;
	WanPacketHandler handler(executor, corrector, dataStore, ring, membership, messenger, peers, sync, logger, callbacks);

	MockSocketWriter sock;
	sock._target = IpAddress("1.2.3.4", 10);
	peers._conn.reset(new PeerConnection);
	membership._ips["1.2.3.4"].reset(new Peer("someguid"));

	// ostensibly, these are file contents for a file write we don't know about yet
	assertTrue( handler.onPacket(sock, formatPacket(32, "foo")) );
	assertTrue( handler.onPacket(sock, formatPacket(32, "")) ); // FIN
	assertTrue( handler.onPacket(sock, formatPacket(32, "bar")) );
	assertEquals( "", dataStore._store["foo"] );

	assertTrue( handler.onPacket(sock, formatPacket(32, "write|name=foo|see") + formatPacket(32, "smell")) );
	string actual = dataStore._store["foo"];
	assertEquals( "seesmellfoobar", actual );
}*/
